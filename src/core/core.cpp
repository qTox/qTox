/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core.h"
#include "src/nexus.h"
#include "src/core/cdata.h"
#include "src/core/cstring.h"
#include "src/persistence/settings.h"
#include "src/widget/gui.h"
#include "src/persistence/historykeeper.h"
#include "src/audio/audio.h"
#include "src/persistence/profilelocker.h"
#include "src/net/avatarbroadcaster.h"
#include "src/persistence/profile.h"
#include "corefile.h"
#include "src/video/camerasource.h"

#include <tox/tox.h>

#include <ctime>
#include <cassert>
#include <limits>
#include <functional>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>
#include <QDateTime>
#include <QList>
#include <QBuffer>
#include <QMutexLocker>

const QString Core::CONFIG_FILE_NAME = "data";
const QString Core::TOX_EXT = ".tox";
QHash<int, ToxGroupCall> Core::groupCalls;
QThread* Core::coreThread{nullptr};

#define MAX_GROUP_MESSAGE_LEN 1024

Core::Core(QThread *CoreThread, Profile& profile) :
    tox(nullptr), toxav(nullptr), profile(profile), ready{false}
{
    coreThread = CoreThread;

    Audio::getInstance();

    videobuf = nullptr;

    toxTimer = new QTimer(this);
    toxTimer->setSingleShot(true);
    connect(toxTimer, &QTimer::timeout, this, &Core::process);
    connect(&Settings::getInstance(), &Settings::dhtServerListChanged, this, &Core::process);

    for (int i=0; i<TOXAV_MAX_CALLS;i++)
    {
        calls[i].active = false;
        calls[i].alSource = 0;
        calls[i].sendAudioTimer = new QTimer();
        calls[i].sendAudioTimer->moveToThread(coreThread);
    }

    // OpenAL init
    QString outDevDescr = Settings::getInstance().getOutDev();
    Audio::openOutput(outDevDescr);
    QString inDevDescr = Settings::getInstance().getInDev();
    Audio::openInput(inDevDescr);
}

void Core::deadifyTox()
{
    if (toxav)
    {
        toxav_kill(toxav);
        toxav = nullptr;
    }
    if (tox)
    {
        tox_kill(tox);
        tox = nullptr;
    }
}

Core::~Core()
{
    qDebug() << "Deleting Core";

    if (coreThread->isRunning())
    {
        if (QThread::currentThread() == coreThread)
            killTimers(false);
        else
            QMetaObject::invokeMethod(this, "killTimers", Qt::BlockingQueuedConnection,
                                      Q_ARG(bool, false));
    }
    coreThread->exit(0);
    while (coreThread->isRunning())
    {
        qApp->processEvents();
        coreThread->wait(500);
    }

    for (ToxCall call : calls)
    {
        if (!call.active)
            continue;
        hangupCall(call.callId);
    }

    deadifyTox();

    delete[] videobuf;
    videobuf=nullptr;

    Audio::closeInput();
    Audio::closeOutput();
}

Core* Core::getInstance()
{
    return Nexus::getCore();
}

void Core::makeTox(QByteArray savedata)
{
    // IPv6 needed for LAN discovery, but can crash some weird routers. On by default, can be disabled in options.
    bool enableIPv6 = Settings::getInstance().getEnableIPv6();
    bool forceTCP = Settings::getInstance().getForceTCP();
    ProxyType proxyType = Settings::getInstance().getProxyType();

    if (enableIPv6)
        qDebug() << "Core starting with IPv6 enabled";
    else
        qWarning() << "Core starting with IPv6 disabled. LAN discovery may not work properly.";

    Tox_Options toxOptions;
    tox_options_default(&toxOptions);
    toxOptions.ipv6_enabled = enableIPv6;
    toxOptions.udp_enabled = !forceTCP;
    toxOptions.start_port = toxOptions.end_port = 0;

    // No proxy by default
    toxOptions.proxy_type = TOX_PROXY_TYPE_NONE;
    toxOptions.proxy_host = nullptr;
    toxOptions.proxy_port = 0;

    toxOptions.savedata_type = (!savedata.isNull() ? TOX_SAVEDATA_TYPE_TOX_SAVE : TOX_SAVEDATA_TYPE_NONE);
    toxOptions.savedata_data = (uint8_t*)savedata.data();
    toxOptions.savedata_length = savedata.size();

    if (proxyType != ProxyType::ptNone)
    {
        QString proxyAddr = Settings::getInstance().getProxyAddr();
        int proxyPort = Settings::getInstance().getProxyPort();

        if (proxyAddr.length() > 255)
        {
            qWarning() << "proxy address" << proxyAddr << "is too long";
        }
        else if (proxyAddr != "" && proxyPort > 0)
        {
            qDebug() << "using proxy" << proxyAddr << ":" << proxyPort;
            // protection against changings in TOX_PROXY_TYPE enum
            if (proxyType == ProxyType::ptSOCKS5)
                toxOptions.proxy_type = TOX_PROXY_TYPE_SOCKS5;
            else if (proxyType == ProxyType::ptHTTP)
                toxOptions.proxy_type = TOX_PROXY_TYPE_HTTP;

            QByteArray proxyAddrData = proxyAddr.toUtf8();
            /// TODO: We're leaking a tiny amount of memory there, go fix that later
            char* proxyAddrCopy = new char[proxyAddrData.size()+1];
            memcpy(proxyAddrCopy, proxyAddrData.data(), proxyAddrData.size()+1);
            toxOptions.proxy_host = proxyAddrCopy;
            toxOptions.proxy_port = proxyPort;
        }
    }

    TOX_ERR_NEW tox_err;
    tox = tox_new(&toxOptions, &tox_err);

    switch (tox_err)
    {
        case TOX_ERR_NEW_OK:
            break;
        case TOX_ERR_NEW_PORT_ALLOC:
            if (enableIPv6)
            {
                toxOptions.ipv6_enabled = false;
                tox = tox_new(&toxOptions, &tox_err);
                if (tox_err == TOX_ERR_NEW_OK)
                {
                    qWarning() << "Core failed to start with IPv6, falling back to IPv4. LAN discovery may not work properly.";
                    break;
                }
            }

            qCritical() << "can't to bind the port";
            emit failedToStart();
            return;
        case TOX_ERR_NEW_PROXY_BAD_HOST:
        case TOX_ERR_NEW_PROXY_BAD_PORT:
            qCritical() << "bad proxy";
            emit badProxy();
            return;
        case TOX_ERR_NEW_PROXY_NOT_FOUND:
            qCritical() << "proxy not found";
            emit badProxy();
            return;
        case TOX_ERR_NEW_LOAD_ENCRYPTED:
            qCritical() << "load data is encrypted";
            emit failedToStart();
            return;
        case TOX_ERR_NEW_LOAD_BAD_FORMAT:
            qCritical() << "bad load data format";
            emit failedToStart();
            return;
        default:
            qCritical() << "Tox core failed to start";
            emit failedToStart();
            return;
    }

    toxav = toxav_new(tox, TOXAV_MAX_CALLS);
    if (toxav == nullptr)
    {
        qCritical() << "Toxav core failed to start";
        emit failedToStart();
        return;
    }
}

void Core::start()
{
    bool isNewProfile = profile.isNewProfile();
    if (isNewProfile)
    {
        qDebug() << "Creating a new profile";
        makeTox(QByteArray());
        setStatusMessage(tr("Toxing on qTox"));
        setUsername(profile.getName());
    }
    else
    {
        qDebug() << "Loading user profile";
        QByteArray savedata = profile.loadToxSave();
        if (savedata.isEmpty())
        {
            emit failedToStart();
            return;
        }
        makeTox(savedata);
    }

    qsrand(time(nullptr));

    if (!tox)
    {
        ready = true;
        GUI::setEnabled(true);
        return;
    }

    // set GUI with user and statusmsg
    QString name = getUsername();
    if (!name.isEmpty())
        emit usernameSet(name);

    QString msg = getStatusMessage();
    if (!msg.isEmpty())
        emit statusMessageSet(msg);

    QString id = getSelfId().toString();
    if (!id.isEmpty())
        emit idSet(id);

    // tox core is already decrypted
    if (Settings::getInstance().getEnableLogging() && Nexus::getProfile()->isEncrypted())
        checkEncryptedHistory();

    loadFriends();

    tox_callback_friend_request(tox, onFriendRequest, this);
    tox_callback_friend_message(tox, onFriendMessage, this);
    tox_callback_friend_name(tox, onFriendNameChange, this);
    tox_callback_friend_typing(tox, onFriendTypingChange, this);
    tox_callback_friend_status_message(tox, onStatusMessageChanged, this);
    tox_callback_friend_status(tox, onUserStatusChanged, this);
    tox_callback_friend_connection_status(tox, onConnectionStatusChanged, this);
    tox_callback_friend_read_receipt(tox, onReadReceiptCallback, this);
    tox_callback_group_invite(tox, onGroupInvite, this);
    tox_callback_group_message(tox, onGroupMessage, this);
    tox_callback_group_namelist_change(tox, onGroupNamelistChange, this);
    tox_callback_group_title(tox, onGroupTitleChange, this);
    tox_callback_group_action(tox, onGroupAction, this);
    tox_callback_file_chunk_request(tox, CoreFile::onFileDataCallback, this);
    tox_callback_file_recv(tox, CoreFile::onFileReceiveCallback, this);
    tox_callback_file_recv_chunk(tox, CoreFile::onFileRecvChunkCallback, this);
    tox_callback_file_recv_control(tox, CoreFile::onFileControlCallback, this);

    toxav_register_callstate_callback(toxav, onAvInvite, av_OnInvite, this);
    toxav_register_callstate_callback(toxav, onAvStart, av_OnStart, this);
    toxav_register_callstate_callback(toxav, onAvCancel, av_OnCancel, this);
    toxav_register_callstate_callback(toxav, onAvReject, av_OnReject, this);
    toxav_register_callstate_callback(toxav, onAvEnd, av_OnEnd, this);
    toxav_register_callstate_callback(toxav, onAvRinging, av_OnRinging, this);
    toxav_register_callstate_callback(toxav, onAvMediaChange, av_OnPeerCSChange, this);
    toxav_register_callstate_callback(toxav, onAvMediaChange, av_OnSelfCSChange, this);
    toxav_register_callstate_callback(toxav, onAvRequestTimeout, av_OnRequestTimeout, this);
    toxav_register_callstate_callback(toxav, onAvPeerTimeout, av_OnPeerTimeout, this);

    toxav_register_audio_callback(toxav, playCallAudio, this);
    toxav_register_video_callback(toxav, playCallVideo, this);

    QPixmap pic = Settings::getInstance().getSavedAvatar(getSelfId().toString());
    if (!pic.isNull() && !pic.size().isEmpty())
    {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        pic.save(&buffer, "PNG");
        buffer.close();
        setAvatar(data);
    }
    else
    {
        qDebug() << "Self avatar not found";
    }

    ready = true;

    // If we created a new profile earlier,
    // now that we're ready save it and ONLY THEN broadcast the new ID.
    // This is useful for e.g. the profileForm that searches for saves.
    if (isNewProfile)
    {
        profile.saveToxSave();
        emit idSet(getSelfId().toString());
    }

    if (isReady())
        GUI::setEnabled(true);

    process(); // starts its own timer
}

/* Using the now commented out statements in checkConnection(), I watched how
 * many ticks disconnects-after-initial-connect lasted. Out of roughly 15 trials,
 * 5 disconnected; 4 were DCd for less than 20 ticks, while the 5th was ~50 ticks.
 * So I set the tolerance here at 25, and initial DCs should be very rare now.
 * This should be able to go to 50 or 100 without affecting legitimate disconnects'
 * downtime, but lets be conservative for now. Edit: now ~~40~~ 30.
 */
#define CORE_DISCONNECT_TOLERANCE 30

void Core::process()
{
    if (!isReady())
        return;

    static int tolerance = CORE_DISCONNECT_TOLERANCE;
    tox_iterate(tox);
    toxav_do(toxav);

#ifdef DEBUG
    //we want to see the debug messages immediately
    fflush(stdout);
#endif

    if (checkConnection())
    {
        tolerance = CORE_DISCONNECT_TOLERANCE;
    }
    else if (!(--tolerance))
    {
        bootstrapDht();
        tolerance = 3*CORE_DISCONNECT_TOLERANCE;
    }

    unsigned sleeptime = qMin(tox_iteration_interval(tox), toxav_do_interval(toxav));
    sleeptime = qMin(sleeptime, CoreFile::corefileIterationInterval());
    toxTimer->start(sleeptime);
}

bool Core::checkConnection()
{
    static bool isConnected = false;
    //static int count = 0;
    bool toxConnected = tox_self_get_connection_status(tox) != TOX_CONNECTION_NONE;

    if (toxConnected && !isConnected)
    {
        qDebug() << "Connected to DHT";
        emit connected();
        isConnected = true;
        //if (count) qDebug() << "disconnect count:" << count;
        //count = 0;
    }
    else if (!toxConnected && isConnected)
    {
        qDebug() << "Disconnected to DHT";
        emit disconnected();
        isConnected = false;
        //count++;
    } //else if (!toxConnected) count++;
    return isConnected;
}

void Core::bootstrapDht()
{
    const Settings& s = Settings::getInstance();
    QList<DhtServer> dhtServerList = s.getDhtServerList();

    int listSize = dhtServerList.size();
    if (listSize == 0)
    {
        qDebug() << "no bootstrap list?!?";
        return;
    }
    static int j = qrand() % listSize;

    qDebug() << "Bootstrapping to the DHT ...";

    int i=0;
    while (i < 2) // i think the more we bootstrap, the more we jitter because the more we overwrite nodes
    {
        const DhtServer& dhtServer = dhtServerList[j % listSize];
        if (tox_bootstrap(tox, dhtServer.address.toLatin1().data(),
            dhtServer.port, CUserId(dhtServer.userId).data(), nullptr))
        {
            qDebug() << "Bootstrapping from " + dhtServer.name
                        + ", addr " + dhtServer.address.toLatin1().data()
                        + ", port " + QString().setNum(dhtServer.port);
        }
        else
        {
            qDebug() << "Error bootstrapping from "+dhtServer.name;
        }

        if (tox_add_tcp_relay(tox, dhtServer.address.toLatin1().data(),
            dhtServer.port, CUserId(dhtServer.userId).data(), nullptr))
        {
            qDebug() << "Adding TCP relay from " + dhtServer.name
                        + ", addr " + dhtServer.address.toLatin1().data()
                        + ", port " + QString().setNum(dhtServer.port);
        }
        else
        {
            qDebug() << "Error adding TCP relay from "+dhtServer.name;
        }

        j++;
        i++;
    }
}

void Core::onFriendRequest(Tox*/* tox*/, const uint8_t* cUserId,
                           const uint8_t* cMessage, size_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendRequestReceived(CUserId::toString(cUserId),
                                                         CString::toString(cMessage, cMessageSize));
}

void Core::onFriendMessage(Tox*/* tox*/, uint32_t friendId, TOX_MESSAGE_TYPE type,
                           const uint8_t* cMessage, size_t cMessageSize, void* core)
{
    bool isAction = (type == TOX_MESSAGE_TYPE_ACTION);
    emit static_cast<Core*>(core)->friendMessageReceived(friendId,CString::toString(cMessage, cMessageSize), isAction);
}

void Core::onFriendNameChange(Tox*/* tox*/, uint32_t friendId,
                              const uint8_t* cName, size_t cNameSize, void* core)
{
    emit static_cast<Core*>(core)->friendUsernameChanged(friendId, CString::toString(cName, cNameSize));
}

void Core::onFriendTypingChange(Tox*/* tox*/, uint32_t friendId, bool isTyping, void *core)
{
    emit static_cast<Core*>(core)->friendTypingChanged(friendId, isTyping ? true : false);
}

void Core::onStatusMessageChanged(Tox*/* tox*/, uint32_t friendId, const uint8_t* cMessage,
                                  size_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendStatusMessageChanged(friendId, CString::toString(cMessage, cMessageSize));
}

void Core::onUserStatusChanged(Tox*/* tox*/, uint32_t friendId, TOX_USER_STATUS userstatus, void* core)
{
    Status status;
    switch (userstatus)
    {
        case TOX_USER_STATUS_NONE:
            status = Status::Online;
            break;
        case TOX_USER_STATUS_AWAY:
            status = Status::Away;
            break;
        case TOX_USER_STATUS_BUSY:
            status = Status::Busy;
            break;
        default:
            status = Status::Online;
            break;
    }

    emit static_cast<Core*>(core)->friendStatusChanged(friendId, status);
}

void Core::onConnectionStatusChanged(Tox*/* tox*/, uint32_t friendId, TOX_CONNECTION status, void* core)
{
    Status friendStatus = status != TOX_CONNECTION_NONE ? Status::Online : Status::Offline;
    emit static_cast<Core*>(core)->friendStatusChanged(friendId, friendStatus);
    if (friendStatus == Status::Offline)
        static_cast<Core*>(core)->checkLastOnline(friendId);
    CoreFile::onConnectionStatusChanged(static_cast<Core*>(core), friendId, friendStatus != Status::Offline);
}

void Core::onGroupAction(Tox*, int groupnumber, int peernumber, const uint8_t *action, uint16_t length, void* _core)
{
    Core* core = static_cast<Core*>(_core);
    emit core->groupMessageReceived(groupnumber, peernumber, CString::toString(action, length), true);
}

void Core::onGroupInvite(Tox*, int32_t friendNumber, uint8_t type, const uint8_t *data, uint16_t length,void *core)
{
    QByteArray pk((char*)data, length);
    if (type == TOX_GROUPCHAT_TYPE_TEXT)
    {
        qDebug() << QString("Text group invite by %1").arg(friendNumber);
        emit static_cast<Core*>(core)->groupInviteReceived(friendNumber,type,pk);
    }
    else if (type == TOX_GROUPCHAT_TYPE_AV)
    {
        qDebug() << QString("AV group invite by %1").arg(friendNumber);
        emit static_cast<Core*>(core)->groupInviteReceived(friendNumber,type,pk);
    }
    else
    {
        qWarning() << "Group invite with unknown type "<<type;
    }
}

void Core::onGroupMessage(Tox*, int groupnumber, int peernumber, const uint8_t * message, uint16_t length, void *_core)
{
    Core* core = static_cast<Core*>(_core);
    emit core->groupMessageReceived(groupnumber, peernumber, CString::toString(message, length), false);
}

void Core::onGroupNamelistChange(Tox*, int groupnumber, int peernumber, uint8_t change, void *core)
{
    qDebug() << QString("Group namelist change %1:%2 %3").arg(groupnumber).arg(peernumber).arg(change);
    emit static_cast<Core*>(core)->groupNamelistChanged(groupnumber, peernumber, change);
}

void Core::onGroupTitleChange(Tox*, int groupnumber, int peernumber, const uint8_t* title, uint8_t len, void* _core)
{
    Core* core = static_cast<Core*>(_core);
    QString author;
    if (peernumber >= 0)
        author = core->getGroupPeerName(groupnumber, peernumber);

    emit core->groupTitleChanged(groupnumber, author, CString::toString(title, len));
}

void Core::onReadReceiptCallback(Tox*, uint32_t friendnumber, uint32_t receipt, void *core)
{
     emit static_cast<Core*>(core)->receiptRecieved(friendnumber, receipt);
}

void Core::acceptFriendRequest(const QString& userId)
{
    uint32_t friendId = tox_friend_add_norequest(tox, CUserId(userId).data(), nullptr);
    if (friendId == std::numeric_limits<uint32_t>::max())
    {
        emit failedToAddFriend(userId);
    }
    else
    {
        profile.saveToxSave();
        emit friendAdded(friendId, userId);
        emit friendshipChanged(friendId);
    }
}

void Core::requestFriendship(const QString& friendAddress, const QString& message)
{
    const QString userId = friendAddress.mid(0, TOX_PUBLIC_KEY_SIZE * 2);

    if (message.isEmpty())
    {
        emit failedToAddFriend(userId, tr("You need to write a message with your request"));
    }
    else if (message.size() > TOX_MAX_FRIEND_REQUEST_LENGTH)
    {
        emit failedToAddFriend(userId, tr("Your message is too long!"));
    }
    else if (hasFriendWithAddress(friendAddress))
    {
        emit failedToAddFriend(userId, tr("Friend is already added"));
    }
    else
    {
        CString cMessage(message);

        uint32_t friendId = tox_friend_add(tox, CFriendAddress(friendAddress).data(),
                                      cMessage.data(), cMessage.size(), nullptr);
        if (friendId == std::numeric_limits<uint32_t>::max())
        {
            qDebug() << "Failed to request friendship";
            emit failedToAddFriend(userId);
        }
        else
        {
            qDebug() << "Requested friendship of "<<friendId;
            // Update our friendAddresses
            Settings::getInstance().updateFriendAdress(friendAddress);
            QString inviteStr = tr("/me offers friendship.");
            if (message.length())
                inviteStr = tr("/me offers friendship, \"%1\"").arg(message);

            HistoryKeeper::getInstance()->addChatEntry(userId, inviteStr, getSelfId().publicKey, QDateTime::currentDateTime(), true);
            emit friendAdded(friendId, userId);
            emit friendshipChanged(friendId);
        }
    }
    profile.saveToxSave();
}

int Core::sendMessage(uint32_t friendId, const QString& message)
{
    QMutexLocker ml(&messageSendMutex);
    CString cMessage(message);
    int receipt = tox_friend_send_message(tox, friendId, TOX_MESSAGE_TYPE_NORMAL,
                                          cMessage.data(), cMessage.size(), nullptr);
    emit messageSentResult(friendId, message, receipt);
    return receipt;
}

int Core::sendAction(uint32_t friendId, const QString &action)
{
    QMutexLocker ml(&messageSendMutex);
    CString cMessage(action);
    int receipt = tox_friend_send_message(tox, friendId, TOX_MESSAGE_TYPE_ACTION,
                                  cMessage.data(), cMessage.size(), nullptr);
    emit messageSentResult(friendId, action, receipt);
    return receipt;
}

void Core::sendTyping(uint32_t friendId, bool typing)
{
    bool ret = tox_self_set_typing(tox, friendId, typing, nullptr);
    if (ret == false)
        emit failedToSetTyping(typing);
}

void Core::sendGroupMessage(int groupId, const QString& message)
{
    QList<CString> cMessages = splitMessage(message, MAX_GROUP_MESSAGE_LEN);

    for (auto &cMsg :cMessages)
    {
        int ret = tox_group_message_send(tox, groupId, cMsg.data(), cMsg.size());

        if (ret == -1)
            emit groupSentResult(groupId, message, ret);
    }
}

void Core::sendGroupAction(int groupId, const QString& message)
{
    QList<CString> cMessages = splitMessage(message, MAX_GROUP_MESSAGE_LEN);

    for (auto &cMsg :cMessages)
    {
        int ret = tox_group_action_send(tox, groupId, cMsg.data(), cMsg.size());

        if (ret == -1)
            emit groupSentResult(groupId, message, ret);
    }
}

void Core::changeGroupTitle(int groupId, const QString& title)
{
    CString cTitle(title);
    int err = tox_group_set_title(tox, groupId, cTitle.data(), cTitle.size());
    if (!err)
        emit groupTitleChanged(groupId, getUsername(), title);
}

void Core::sendFile(uint32_t friendId, QString Filename, QString FilePath, long long filesize)
{
    CoreFile::sendFile(this, friendId, Filename, FilePath, filesize);
}

void Core::sendAvatarFile(uint32_t friendId, const QByteArray& data)
{
    CoreFile::sendAvatarFile(this, friendId, data);
}

void Core::pauseResumeFileSend(uint32_t friendId, uint32_t fileNum)
{
    CoreFile::pauseResumeFileSend(this, friendId, fileNum);
}

void Core::pauseResumeFileRecv(uint32_t friendId, uint32_t fileNum)
{
    CoreFile::pauseResumeFileRecv(this, friendId, fileNum);
}

void Core::cancelFileSend(uint32_t friendId, uint32_t fileNum)
{
    CoreFile::cancelFileSend(this, friendId, fileNum);
}

void Core::cancelFileRecv(uint32_t friendId, uint32_t fileNum)
{
    CoreFile::cancelFileRecv(this, friendId, fileNum);
}

void Core::rejectFileRecvRequest(uint32_t friendId, uint32_t fileNum)
{
    CoreFile::rejectFileRecvRequest(this, friendId, fileNum);
}

void Core::acceptFileRecvRequest(uint32_t friendId, uint32_t fileNum, QString path)
{
    CoreFile::acceptFileRecvRequest(this, friendId, fileNum, path);
}

void Core::removeFriend(uint32_t friendId, bool fake)
{
    if (!isReady() || fake)
        return;

    if (tox_friend_delete(tox, friendId, nullptr) == false)
    {
        emit failedToRemoveFriend(friendId);
    }
    else
    {
        profile.saveToxSave();
        emit friendRemoved(friendId);
    }
}

void Core::removeGroup(int groupId, bool fake)
{
    if (!isReady() || fake)
        return;

    tox_del_groupchat(tox, groupId);

    if (groupCalls[groupId].active)
        leaveGroupCall(groupId);
}

QString Core::getUsername() const
{
    QString sname;
    int size = tox_self_get_name_size(tox);
    uint8_t* name = new uint8_t[size];
    tox_self_get_name(tox, name);
    sname = CString::toString(name, size);
    delete[] name;
    return sname;
}

void Core::setUsername(const QString& username)
{
    CString cUsername(username);

    if (tox_self_set_name(tox, cUsername.data(), cUsername.size(), nullptr) == false)
    {
        emit failedToSetUsername(username);
    }
    else
    {
        emit usernameSet(username);
        if (ready)
            profile.saveToxSave();
    }
}

void Core::setAvatar(const QByteArray& data)
{
    QPixmap pic;
    pic.loadFromData(data);
    Settings::getInstance().saveAvatar(pic, getSelfId().toString());
    emit selfAvatarChanged(pic);

    AvatarBroadcaster::setAvatar(data);
    AvatarBroadcaster::enableAutoBroadcast();
}

ToxId Core::getSelfId() const
{
    uint8_t friendAddress[TOX_ADDRESS_SIZE] = {0};
    tox_self_get_address(tox, friendAddress);
    return ToxId(CFriendAddress::toString(friendAddress));
}

QString Core::getIDString() const
{
    return getSelfId().toString().left(12);
    // 12 is the smallest multiple of four such that
    // 16^n > 10^10 (which is roughly the planet's population)
}

QPair<QByteArray, QByteArray> Core::getKeypair() const
{
    QPair<QByteArray, QByteArray> keypair;
    if (!tox)
        return keypair;

    char buf[std::max(TOX_PUBLIC_KEY_SIZE, TOX_SECRET_KEY_SIZE)];
    tox_self_get_public_key(tox, (uint8_t*)buf);
    keypair.first = QByteArray(buf, TOX_PUBLIC_KEY_SIZE);
    tox_self_get_secret_key(tox, (uint8_t*)buf);
    keypair.second = QByteArray(buf, TOX_SECRET_KEY_SIZE);
    return keypair;
}

QString Core::getStatusMessage() const
{
    QString sname;
    size_t size = tox_self_get_status_message_size(tox);
    uint8_t* name = new uint8_t[size];
    tox_self_get_status_message(tox, name);
    sname = CString::toString(name, size);
    delete[] name;
    return sname;
}

void Core::setStatusMessage(const QString& message)
{
    CString cMessage(message);

    if (tox_self_set_status_message(tox, cMessage.data(), cMessage.size(), nullptr) == false)
    {
        emit failedToSetStatusMessage(message);
    }
    else
    {
        if (ready)
            profile.saveToxSave();
        emit statusMessageSet(message);
    }
}

void Core::setStatus(Status status)
{
    TOX_USER_STATUS userstatus;
    switch (status)
    {
        case Status::Online:
            userstatus = TOX_USER_STATUS_NONE;
            break;
        case Status::Away:
            userstatus = TOX_USER_STATUS_AWAY;
            break;
        case Status::Busy:
            userstatus = TOX_USER_STATUS_BUSY;
            break;
        default:
            return;
            break;
    }

    tox_self_set_status(tox, userstatus);
    profile.saveToxSave();
    emit statusSet(status);
}

QString Core::sanitize(QString name)
{
    // these are pretty much Windows banned filename characters
    QList<QChar> banned = {'/', '\\', ':', '<', '>', '"', '|', '?', '*'};
    for (QChar c : banned)
        name.replace(c, '_');

    // also remove leading and trailing periods
    if (name[0] == '.')
        name[0] = '_';

    if (name.endsWith('.'))
        name[name.length()-1] = '_';

    return name;
}

QByteArray Core::getToxSaveData()
{
    uint32_t fileSize = tox_get_savedata_size(tox);
    QByteArray data;
    data.resize(fileSize);
    tox_get_savedata(tox, (uint8_t*)data.data());
    return data;
}

void Core::loadFriends()
{
    const uint32_t friendCount = tox_self_get_friend_list_size(tox);
    if (friendCount > 0)
    {
        // assuming there are not that many friends to fill up the whole stack
        uint32_t *ids = new uint32_t[friendCount];
        tox_self_get_friend_list(tox, ids);
        uint8_t clientId[TOX_PUBLIC_KEY_SIZE];
        for (int32_t i = 0; i < static_cast<int32_t>(friendCount); ++i)
        {
            if (tox_friend_get_public_key(tox, ids[i], clientId, nullptr))
            {
                emit friendAdded(ids[i], CUserId::toString(clientId));

                const size_t nameSize = tox_friend_get_name_size(tox, ids[i], nullptr);
                if (nameSize && nameSize != SIZE_MAX)
                {
                    uint8_t *name = new uint8_t[nameSize];
                    if (tox_friend_get_name(tox, ids[i], name, nullptr))
                        emit friendUsernameChanged(ids[i], CString::toString(name, nameSize));
                    delete[] name;
                }

                const size_t statusMessageSize = tox_friend_get_status_message_size(tox, ids[i], nullptr);
                if (statusMessageSize != SIZE_MAX)
                {
                    uint8_t *statusMessage = new uint8_t[statusMessageSize];
                    if (tox_friend_get_status_message(tox, ids[i], statusMessage, nullptr))
                    {
                        emit friendStatusMessageChanged(ids[i], CString::toString(statusMessage, statusMessageSize));
                    }
                    delete[] statusMessage;
                }

                checkLastOnline(ids[i]);
            }

        }
        delete[] ids;
    }
}

void Core::checkLastOnline(uint32_t friendId) {
    const uint64_t lastOnline = tox_friend_get_last_online(tox, friendId, nullptr);
    if (lastOnline != std::numeric_limits<uint64_t>::max())
        emit friendLastSeenChanged(friendId, QDateTime::fromTime_t(lastOnline));
}

QVector<uint32_t> Core::getFriendList() const
{
    QVector<uint32_t> friends;
    friends.resize(tox_self_get_friend_list_size(tox));
    tox_self_get_friend_list(tox, friends.data());
    return friends;
}

int Core::getGroupNumberPeers(int groupId) const
{
    return tox_group_number_peers(tox, groupId);
}

QString Core::getGroupPeerName(int groupId, int peerId) const
{
    QString name;
    uint8_t nameArray[TOX_MAX_NAME_LENGTH];
    int length = tox_group_peername(tox, groupId, peerId, nameArray);
    if (length == -1)
    {
        qWarning() << "getGroupPeerName: Unknown error";
        return name;
    }
    name = CString::toString(nameArray, length);
    return name;
}

ToxId Core::getGroupPeerToxId(int groupId, int peerId) const
{
    ToxId peerToxId;

    uint8_t rawID[TOX_PUBLIC_KEY_SIZE];
    int res = tox_group_peer_pubkey(tox, groupId, peerId, rawID);
    if (res == -1)
    {
        qWarning() << "getGroupPeerToxId: Unknown error";
        return peerToxId;
    }

    peerToxId = ToxId(CUserId::toString(rawID));
    return peerToxId;
}

QList<QString> Core::getGroupPeerNames(int groupId) const
{
    QList<QString> names;
    int nPeers = getGroupNumberPeers(groupId);
    if (nPeers == -1)
    {
        qWarning() << "getGroupPeerNames: Unable to get number of peers";
        return names;
    }
    uint8_t namesArray[nPeers][TOX_MAX_NAME_LENGTH];
    uint16_t* lengths = new uint16_t[nPeers];
    int result = tox_group_get_names(tox, groupId, namesArray, lengths, nPeers);
    if (result != nPeers)
    {
        qWarning() << "getGroupPeerNames: Unexpected result";
        return names;
    }
    for (int i=0; i<nPeers; i++)
       names.push_back(CString::toString(namesArray[i], lengths[i]));

    return names;
}

int Core::joinGroupchat(int32_t friendnumber, uint8_t type, const uint8_t* friend_group_public_key,uint16_t length) const
{
    if (type == TOX_GROUPCHAT_TYPE_TEXT)
    {
        qDebug() << QString("Trying to join text groupchat invite sent by friend %1").arg(friendnumber);
        return tox_join_groupchat(tox, friendnumber, friend_group_public_key,length);
    }
    else if (type == TOX_GROUPCHAT_TYPE_AV)
    {
        qDebug() << QString("Trying to join AV groupchat invite sent by friend %1").arg(friendnumber);
        return toxav_join_av_groupchat(tox, friendnumber, friend_group_public_key, length,
                                       &Audio::playGroupAudioQueued, const_cast<Core*>(this));
    }
    else
    {
        qWarning() << "joinGroupchat: Unknown groupchat type "<<type;
        return -1;
    }
}

void Core::quitGroupChat(int groupId) const
{
    tox_del_groupchat(tox, groupId);
}

void Core::groupInviteFriend(uint32_t friendId, int groupId)
{
    tox_invite_friend(tox, friendId, groupId);
}

void Core::createGroup(uint8_t type)
{
    if (type == TOX_GROUPCHAT_TYPE_TEXT)
    {
        emit emptyGroupCreated(tox_add_groupchat(tox));
    }
    else if (type == TOX_GROUPCHAT_TYPE_AV)
    {
        emit emptyGroupCreated(toxav_add_av_groupchat(tox, &Audio::playGroupAudioQueued, this));
    }
    else
    {
        qWarning() << "createGroup: Unknown type "<<type;
    }
}

bool Core::isGroupAvEnabled(int groupId)
{
    return tox_group_get_type(tox, groupId) == TOX_GROUPCHAT_TYPE_AV;
}

bool Core::isFriendOnline(uint32_t friendId) const
{
    return tox_friend_get_connection_status(tox, friendId, nullptr) != TOX_CONNECTION_NONE;
}

bool Core::hasFriendWithAddress(const QString &addr) const
{
    // Valid length check
    if (addr.length() != (TOX_ADDRESS_SIZE * 2))
    {
        return false;
    }

    QString pubkey = addr.left(TOX_PUBLIC_KEY_SIZE * 2);
    return hasFriendWithPublicKey(pubkey);
}

bool Core::hasFriendWithPublicKey(const QString &pubkey) const
{
    // Valid length check
    if (pubkey.length() != (TOX_PUBLIC_KEY_SIZE * 2))
        return false;

    bool found = false;
    const size_t friendCount = tox_self_get_friend_list_size(tox);
    if (friendCount > 0)
    {
        uint32_t *ids = new uint32_t[friendCount];
        tox_self_get_friend_list(tox, ids);
        for (int32_t i = 0; i < static_cast<int32_t>(friendCount); ++i)
        {
            // getFriendAddress may return either id (public key) or address
            QString addrOrId = getFriendAddress(ids[i]);

            // Set true if found
            if (addrOrId.toUpper().startsWith(pubkey.toUpper()))
            {
                found = true;
                break;
            }
        }

        delete[] ids;
    }

    return found;
}

QString Core::getFriendAddress(uint32_t friendNumber) const
{
    // If we don't know the full address of the client, return just the id, otherwise get the full address
    uint8_t rawid[TOX_PUBLIC_KEY_SIZE];
    if (!tox_friend_get_public_key(tox, friendNumber, rawid, nullptr))
    {
        qWarning() << "getFriendAddress: Getting public key failed";
        return QString();
    }
    QByteArray data((char*)rawid,TOX_PUBLIC_KEY_SIZE);
    QString id = data.toHex().toUpper();

    QString addr = Settings::getInstance().getFriendAdress(id);
    if (addr.size() > id.size())
        return addr;

    return id;
}

QString Core::getFriendUsername(uint32_t friendnumber) const
{
    size_t namesize = tox_friend_get_name_size(tox, friendnumber, nullptr);
    if (namesize == SIZE_MAX)
    {
        qWarning() << "getFriendUsername: Failed to get name size for friend "<<friendnumber;
        return QString();
    }
    uint8_t* name = new uint8_t[namesize];
    tox_friend_get_name(tox, friendnumber, name, nullptr);
    QString sname = CString::toString(name, namesize);
    delete[] name;
    return sname;
}

QList<CString> Core::splitMessage(const QString &message, int maxLen)
{
    QList<CString> splittedMsgs;
    QByteArray ba_message(message.toUtf8());

    while (ba_message.size() > maxLen)
    {
        int splitPos = ba_message.lastIndexOf(' ', maxLen - 1);
        if (splitPos <= 0)
        {
            splitPos = maxLen;
            if (ba_message[splitPos] & 0x80)
            {
                do {
                    splitPos--;
                } while (!(ba_message[splitPos] & 0x40));
            }
            splitPos--;
        }

        splittedMsgs.push_back(CString(ba_message.left(splitPos + 1)));
        ba_message = ba_message.mid(splitPos + 1);
    }

    splittedMsgs.push_back(CString(ba_message));

    return splittedMsgs;
}

QString Core::getPeerName(const ToxId& id) const
{
    QString name;
    CUserId cid(id.toString());

    uint32_t friendId = tox_friend_by_public_key(tox, (uint8_t*)cid.data(), nullptr);
    if (friendId == std::numeric_limits<uint32_t>::max())
    {
        qWarning() << "getPeerName: No such peer";
        return name;
    }

    const size_t nameSize = tox_friend_get_name_size(tox, friendId, nullptr);
    if (nameSize == SIZE_MAX)
        return name;

    uint8_t* cname = new uint8_t[nameSize<TOX_MAX_NAME_LENGTH ? TOX_MAX_NAME_LENGTH : nameSize];
    if (tox_friend_get_name(tox, friendId, cname, nullptr) == false)
    {
        qWarning() << "getPeerName: Can't get name of friend "+QString().setNum(friendId);
        delete[] cname;
        return name;
    }

    name = name.fromLocal8Bit((char*)cname, nameSize);
    delete[] cname;
    return name;
}

bool Core::isReady()
{
    return toxav && tox && ready;
}

void Core::setNospam(uint32_t nospam)
{
    uint8_t *nspm = reinterpret_cast<uint8_t*>(&nospam);
    std::reverse(nspm, nspm + 4);
    tox_self_set_nospam(tox, nospam);
}

void Core::resetCallSources()
{
    for (ToxGroupCall& call : groupCalls)
    {
        for (ALuint source : call.alSources)
            alDeleteSources(1, &source);
        call.alSources.clear();
    }

    for (ToxCall& call : calls)
    {
        if (call.active && call.alSource)
        {
            ALuint tmp = call.alSource;
            call.alSource = 0;
            alDeleteSources(1, &tmp);

            alGenSources(1, &call.alSource);
        }
    }
}

void Core::killTimers(bool onlyStop)
{
    assert(QThread::currentThread() == coreThread);
    toxTimer->stop();
    if (!onlyStop)
    {
        delete toxTimer;
        toxTimer = nullptr;
    }
}

void Core::reset()
{
    assert(QThread::currentThread() == coreThread);

    ready = false;
    killTimers(true);
    deadifyTox();

    emit selfAvatarChanged(QPixmap(":/img/contact_dark.svg"));
    GUI::clearContacts();

    start();
}
