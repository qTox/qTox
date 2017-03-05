/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2015 by The qTox Project Contributors

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
#include "corefile.h"
#include "src/core/coreav.h"
#include "src/core/toxstring.h"
#include "src/net/avatarbroadcaster.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/profilelocker.h"
#include "src/persistence/settings.h"
#include "src/video/camerasource.h"
#include "src/widget/gui.h"

#include <tox/tox.h>
#include <tox/toxav.h>

#include <cassert>
#include <ctime>
#include <functional>
#include <limits>

#include <QBuffer>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QList>
#include <QMutexLocker>
#include <QSaveFile>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>

const QString Core::CONFIG_FILE_NAME = "data";
const QString Core::TOX_EXT = ".tox";
QThread* Core::coreThread{nullptr};

#define MAX_GROUP_MESSAGE_LEN 1024

Core::Core(QThread* CoreThread, Profile& profile)
    : tox(nullptr)
    , av(nullptr)
    , profile(profile)
    , ready{false}
{
    coreThread = CoreThread;

    toxTimer = new QTimer(this);
    toxTimer->setSingleShot(true);
    connect(toxTimer, &QTimer::timeout, this, &Core::process);
    connect(&Settings::getInstance(), &Settings::dhtServerListChanged, this, &Core::process);
}

void Core::deadifyTox()
{
    if (av) {
        delete av;
        av = nullptr;
    }
    if (tox) {
        tox_kill(tox);
        tox = nullptr;
    }
}

Core::~Core()
{
    if (coreThread->isRunning()) {
        if (QThread::currentThread() == coreThread)
            killTimers(false);
        else
            QMetaObject::invokeMethod(this, "killTimers", Qt::BlockingQueuedConnection,
                                      Q_ARG(bool, false));
    }
    coreThread->exit(0);
    if (QThread::currentThread() != coreThread) {
        while (coreThread->isRunning()) {
            qApp->processEvents();
            coreThread->wait(500);
        }
    }

    deadifyTox();
}

/**
 * @brief Returns the global widget's Core instance
 */
Core* Core::getInstance()
{
    return Nexus::getCore();
}

const CoreAV* Core::getAv() const
{
    return av;
}

CoreAV* Core::getAv()
{
    return av;
}

void Core::makeTox(QByteArray savedata)
{
    // IPv6 needed for LAN discovery, but can crash some weird routers. On by default, can be
    // disabled in options.
    bool enableIPv6 = Settings::getInstance().getEnableIPv6();
    bool forceTCP = Settings::getInstance().getForceTCP();
    Settings::ProxyType proxyType = Settings::getInstance().getProxyType();
    quint16 proxyPort = Settings::getInstance().getProxyPort();
    QString proxyAddr = Settings::getInstance().getProxyAddr();
    QByteArray proxyAddrData = proxyAddr.toUtf8();

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

    toxOptions.savedata_type =
        (!savedata.isNull() ? TOX_SAVEDATA_TYPE_TOX_SAVE : TOX_SAVEDATA_TYPE_NONE);
    toxOptions.savedata_data = (uint8_t*)savedata.data();
    toxOptions.savedata_length = savedata.size();

    if (proxyType != Settings::ProxyType::ptNone) {
        if (proxyAddr.length() > 255) {
            qWarning() << "proxy address" << proxyAddr << "is too long";
        } else if (proxyAddr != "" && proxyPort > 0) {
            qDebug() << "using proxy" << proxyAddr << ":" << proxyPort;
            // protection against changings in TOX_PROXY_TYPE enum
            if (proxyType == Settings::ProxyType::ptSOCKS5)
                toxOptions.proxy_type = TOX_PROXY_TYPE_SOCKS5;
            else if (proxyType == Settings::ProxyType::ptHTTP)
                toxOptions.proxy_type = TOX_PROXY_TYPE_HTTP;

            toxOptions.proxy_host = proxyAddrData.data();
            toxOptions.proxy_port = proxyPort;
        }
    }

    TOX_ERR_NEW tox_err;
    tox = tox_new(&toxOptions, &tox_err);

    switch (tox_err) {
    case TOX_ERR_NEW_OK:
        break;
    case TOX_ERR_NEW_LOAD_BAD_FORMAT:
        qWarning() << "failed to parse Tox save data";
        break;
    case TOX_ERR_NEW_PORT_ALLOC:
        if (enableIPv6) {
            toxOptions.ipv6_enabled = false;
            tox = tox_new(&toxOptions, &tox_err);
            if (tox_err == TOX_ERR_NEW_OK) {
                qWarning() << "Core failed to start with IPv6, falling back to IPv4. LAN discovery "
                              "may not work properly.";
                break;
            }
        }

        qCritical() << "can't to bind the port";
        emit failedToStart();
        return;
    case TOX_ERR_NEW_PROXY_BAD_HOST:
    case TOX_ERR_NEW_PROXY_BAD_PORT:
    case TOX_ERR_NEW_PROXY_BAD_TYPE:
        qCritical() << "bad proxy, error code:" << tox_err;
        emit badProxy();
        return;
    case TOX_ERR_NEW_PROXY_NOT_FOUND:
        qCritical() << "proxy not found";
        emit badProxy();
        return;
    case TOX_ERR_NEW_LOAD_ENCRYPTED:
        qCritical() << "attempted to load encrypted Tox save data";
        emit failedToStart();
        return;
    case TOX_ERR_NEW_MALLOC:
        qCritical() << "memory allocation failed";
        emit failedToStart();
        return;
    case TOX_ERR_NEW_NULL:
        qCritical() << "a parameter was null";
        emit failedToStart();
        return;
    default:
        qCritical() << "Tox core failed to start, unknown error code:" << tox_err;
        emit failedToStart();
        return;
    }

    av = new CoreAV(tox);
    if (av->getToxAv() == nullptr) {
        qCritical() << "Toxav core failed to start";
        emit failedToStart();
        return;
    }
}

/**
 * @brief Initializes the core, must be called before anything else
 */
void Core::start()
{
    bool isNewProfile = profile.isNewProfile();
    if (isNewProfile) {
        qDebug() << "Creating a new profile";
        makeTox(QByteArray());
        setStatusMessage(tr("Toxing on qTox"));
        setUsername(profile.getName());
    } else {
        qDebug() << "Loading user profile";
        QByteArray savedata = profile.loadToxSave();
        if (savedata.isEmpty()) {
            emit failedToStart();
            return;
        }
        makeTox(savedata);
    }

    qsrand(time(nullptr));

    if (!tox) {
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

    ToxId id = getSelfId();
    if (id.isValid()) // TODO: probably useless check, comes basically directly from toxcore
        emit idSet(id);

    loadFriends();

    tox_callback_friend_request(tox, onFriendRequest);
    tox_callback_friend_message(tox, onFriendMessage);
    tox_callback_friend_name(tox, onFriendNameChange);
    tox_callback_friend_typing(tox, onFriendTypingChange);
    tox_callback_friend_status_message(tox, onStatusMessageChanged);
    tox_callback_friend_status(tox, onUserStatusChanged);
    tox_callback_friend_connection_status(tox, onConnectionStatusChanged);
    tox_callback_friend_read_receipt(tox, onReadReceiptCallback);
    tox_callback_conference_invite(tox, onGroupInvite);
    tox_callback_conference_message(tox, onGroupMessage);
    tox_callback_conference_namelist_change(tox, onGroupNamelistChange);
    tox_callback_conference_title(tox, onGroupTitleChange);
    tox_callback_file_chunk_request(tox, CoreFile::onFileDataCallback);
    tox_callback_file_recv(tox, CoreFile::onFileReceiveCallback);
    tox_callback_file_recv_chunk(tox, CoreFile::onFileRecvChunkCallback);
    tox_callback_file_recv_control(tox, CoreFile::onFileControlCallback);

    QPixmap pic = profile.loadAvatar();
    if (!pic.isNull() && !pic.size().isEmpty()) {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        pic.save(&buffer, "PNG");
        buffer.close();
        setAvatar(data);
    } else {
        qDebug() << "Self avatar not found, will broadcast empty avatar to friends";
        setAvatar({});
    }

    ready = true;

    if (isNewProfile) {
        profile.saveToxSave();
    }

    if (isReady())
        GUI::setEnabled(true);

    process(); // starts its own timer
    av->start();
}

/* Using the now commented out statements in checkConnection(), I watched how
 * many ticks disconnects-after-initial-connect lasted. Out of roughly 15 trials,
 * 5 disconnected; 4 were DCd for less than 20 ticks, while the 5th was ~50 ticks.
 * So I set the tolerance here at 25, and initial DCs should be very rare now.
 * This should be able to go to 50 or 100 without affecting legitimate disconnects'
 * downtime, but lets be conservative for now. Edit: now ~~40~~ 30.
 */
#define CORE_DISCONNECT_TOLERANCE 30

/**
 * @brief Processes toxcore events and ensure we stay connected, called by its own timer
 */
void Core::process()
{
    if (!isReady()) {
        av->stop();
        return;
    }

    static int tolerance = CORE_DISCONNECT_TOLERANCE;
    tox_iterate(tox, getInstance());

#ifdef DEBUG
    // we want to see the debug messages immediately
    fflush(stdout);
#endif

    if (checkConnection()) {
        tolerance = CORE_DISCONNECT_TOLERANCE;
    } else if (!(--tolerance)) {
        bootstrapDht();
        tolerance = 3 * CORE_DISCONNECT_TOLERANCE;
    }

    unsigned sleeptime = qMin(tox_iteration_interval(tox), CoreFile::corefileIterationInterval());
    toxTimer->start(sleeptime);
}

bool Core::checkConnection()
{
    static bool isConnected = false;
    // static int count = 0;
    bool toxConnected = tox_self_get_connection_status(tox) != TOX_CONNECTION_NONE;

    if (toxConnected && !isConnected) {
        qDebug() << "Connected to the DHT";
        emit connected();
        isConnected = true;
        // if (count) qDebug() << "disconnect count:" << count;
        // count = 0;
    } else if (!toxConnected && isConnected) {
        qDebug() << "Disconnected from the DHT";
        emit disconnected();
        isConnected = false;
        // count++;
    } // else if (!toxConnected) ++count;
    return isConnected;
}

/**
 * @brief Connects us to the Tox network
 */
void Core::bootstrapDht()
{
    const Settings& s = Settings::getInstance();
    QList<DhtServer> dhtServerList = s.getDhtServerList();

    int listSize = dhtServerList.size();
    if (listSize == 0) {
        qWarning() << "no bootstrap list?!?";
        return;
    }
    static int j = qrand() % listSize;

    int i = 0;
    while (i < 2) // i think the more we bootstrap, the more we jitter because the more we overwrite
                  // nodes
    {
        const DhtServer& dhtServer = dhtServerList[j % listSize];
        qDebug() << "Connecting to " + QString(dhtServer.address.toLatin1().data()) + ':'
                        + QString().setNum(dhtServer.port) + " (" + dhtServer.name + ')';

        QByteArray address = dhtServer.address.toLatin1();
        // TODO: constucting the pk via ToxId is a workaround
        ToxPk pk = ToxId{dhtServer.userId}.getPublicKey();

        const uint8_t* pkPtr = reinterpret_cast<const uint8_t*>(pk.getBytes());

        if (!tox_bootstrap(tox, address.constData(), dhtServer.port, pkPtr, nullptr)) {
            qDebug() << "Error bootstrapping from " + dhtServer.name;
        }

        if (!tox_add_tcp_relay(tox, address.constData(), dhtServer.port, pkPtr, nullptr)) {
            qDebug() << "Error adding TCP relay from " + dhtServer.name;
        }

        ++j;
        ++i;
    }
}

void Core::onFriendRequest(Tox* /* tox*/, const uint8_t* cFriendPk, const uint8_t* cMessage,
                           size_t cMessageSize, void* core)
{
    ToxPk friendPk(cFriendPk);
    emit static_cast<Core*>(core)->friendRequestReceived(friendPk,
                                                         ToxString(cMessage, cMessageSize).getQString());
}

void Core::onFriendMessage(Tox* /* tox*/, uint32_t friendId, TOX_MESSAGE_TYPE type,
                           const uint8_t* cMessage, size_t cMessageSize, void* core)
{
    bool isAction = (type == TOX_MESSAGE_TYPE_ACTION);
    emit static_cast<Core*>(core)->friendMessageReceived(friendId,
                                                         ToxString(cMessage, cMessageSize).getQString(),
                                                         isAction);
}

void Core::onFriendNameChange(Tox* /* tox*/, uint32_t friendId, const uint8_t* cName,
                              size_t cNameSize, void* core)
{
    emit static_cast<Core*>(core)->friendUsernameChanged(friendId, ToxString(cName, cNameSize).getQString());
}

void Core::onFriendTypingChange(Tox* /* tox*/, uint32_t friendId, bool isTyping, void* core)
{
    emit static_cast<Core*>(core)->friendTypingChanged(friendId, isTyping ? true : false);
}

void Core::onStatusMessageChanged(Tox* /* tox*/, uint32_t friendId, const uint8_t* cMessage,
                                  size_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendStatusMessageChanged(friendId,
                                                              ToxString(cMessage, cMessageSize).getQString());
}

void Core::onUserStatusChanged(Tox* /* tox*/, uint32_t friendId, TOX_USER_STATUS userstatus, void* core)
{
    Status status;
    switch (userstatus) {
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

void Core::onConnectionStatusChanged(Tox* /* tox*/, uint32_t friendId, TOX_CONNECTION status, void* core)
{
    Status friendStatus = status != TOX_CONNECTION_NONE ? Status::Online : Status::Offline;
    // Ignore Online because it will be emited from onUserStatusChanged
    if (friendStatus == Status::Offline) {
        emit static_cast<Core*>(core)->friendStatusChanged(friendId, friendStatus);
        static_cast<Core*>(core)->checkLastOnline(friendId);
        CoreFile::onConnectionStatusChanged(static_cast<Core*>(core), friendId, 
                friendStatus != Status::Offline);
    }
}

void Core::onGroupInvite(Tox*, uint32_t friendId, TOX_CONFERENCE_TYPE type, const uint8_t* data,
                         size_t length, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    QByteArray pk((char*)data, length);
    if (type == TOX_CONFERENCE_TYPE_TEXT) {
        qDebug() << QString("Text group invite by %1").arg(friendId);
        emit core->groupInviteReceived(friendId, type, pk);
    } else if (type == TOX_CONFERENCE_TYPE_AV) {
        qDebug() << QString("AV group invite by %1").arg(friendId);
        emit core->groupInviteReceived(friendId, type, pk);
    } else {
        qWarning() << "Group invite with unknown type " << type;
    }
}

void Core::onGroupMessage(Tox*, uint32_t groupId, uint32_t peerId, TOX_MESSAGE_TYPE type,
                          const uint8_t* cMessage, size_t length, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    bool isAction = type == TOX_MESSAGE_TYPE_ACTION;
    emit core->groupMessageReceived(groupId, peerId, ToxString(cMessage, length).getQString(), isAction);
}

void Core::onGroupNamelistChange(Tox*, uint32_t groupId, uint32_t peerId,
                                 TOX_CONFERENCE_STATE_CHANGE change, void* core)
{
    CoreAV* coreAv = static_cast<Core*>(core)->getAv();
    if ((change == TOX_CONFERENCE_STATE_CHANGE_PEER_EXIT) && (coreAv->isGroupAvEnabled(groupId))) {
        CoreAV::invalidateGroupCallPeerSource(groupId, peerId);
    }
    qDebug() << QString("Group namelist change %1:%2 %3").arg(groupId).arg(peerId).arg(change);
    emit static_cast<Core*>(core)->groupNamelistChanged(groupId, peerId, change);
}

void Core::onGroupTitleChange(Tox*, uint32_t groupId, uint32_t peerId, const uint8_t* cTitle,
                              size_t length, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    QString author = core->getGroupPeerName(groupId, peerId);
    emit core->groupTitleChanged(groupId, author, ToxString(cTitle, length).getQString());
}

void Core::onReadReceiptCallback(Tox*, uint32_t friendId, uint32_t receipt, void* core)
{
    emit static_cast<Core*>(core)->receiptRecieved(friendId, receipt);
}

void Core::acceptFriendRequest(const ToxPk& friendPk)
{
    // TODO: error handling
    uint32_t friendId = tox_friend_add_norequest(tox, friendPk.getBytes(), nullptr);
    if (friendId == std::numeric_limits<uint32_t>::max()) {
        emit failedToAddFriend(friendPk);
    } else {
        profile.saveToxSave();
        emit friendAdded(friendId, friendPk);
        emit friendshipChanged(friendId);
    }
}

void Core::requestFriendship(const ToxId& friendAddress, const QString& message)
{
    ToxPk friendPk = friendAddress.getPublicKey();

    if (!friendAddress.isValid()) {
        emit failedToAddFriend(friendPk, tr("Invalid Tox ID"));
    } else if (message.isEmpty()) {
        emit failedToAddFriend(friendPk, tr("You need to write a message with your request"));
    } else if (message.size() > TOX_MAX_FRIEND_REQUEST_LENGTH) {
        emit failedToAddFriend(friendPk, tr("Your message is too long!"));
    } else if (hasFriendWithPublicKey(friendPk)) {
        emit failedToAddFriend(friendPk, tr("Friend is already added"));
    } else {
        ToxString cMessage(message);

        uint32_t friendId =
            tox_friend_add(tox, friendAddress.getBytes(), cMessage.data(), cMessage.size(), nullptr);
        if (friendId == std::numeric_limits<uint32_t>::max()) {
            qDebug() << "Failed to request friendship";
            emit failedToAddFriend(friendPk);
        } else {
            qDebug() << "Requested friendship of " << friendId;
            // Update our friendAddresses
            Settings::getInstance().updateFriendAddress(friendAddress.toString());

            // TODO: start: this really shouldn't be in Core
            QString inviteStr = tr("/me offers friendship.");
            if (message.length())
                inviteStr = tr("/me offers friendship, \"%1\"").arg(message);

            Profile* profile = Nexus::getProfile();
            if (profile->isHistoryEnabled()) {
                profile->getHistory()->addNewMessage(friendAddress.toString(), inviteStr,
                                                     getSelfId().getPublicKey().toString(),
                                                     QDateTime::currentDateTime(), true, QString());
            }
            // TODO: end

            emit friendAdded(friendId, friendAddress.getPublicKey());
            emit friendshipChanged(friendId);
        }
    }
    profile.saveToxSave();
}

int Core::sendMessage(uint32_t friendId, const QString& message)
{
    QMutexLocker ml(&messageSendMutex);
    ToxString cMessage(message);
    int receipt = tox_friend_send_message(tox, friendId, TOX_MESSAGE_TYPE_NORMAL, cMessage.data(),
                                          cMessage.size(), nullptr);
    emit messageSentResult(friendId, message, receipt);
    return receipt;
}

int Core::sendAction(uint32_t friendId, const QString& action)
{
    QMutexLocker ml(&messageSendMutex);
    ToxString cMessage(action);
    int receipt = tox_friend_send_message(tox, friendId, TOX_MESSAGE_TYPE_ACTION, cMessage.data(),
                                          cMessage.size(), nullptr);
    emit messageSentResult(friendId, action, receipt);
    return receipt;
}

void Core::sendTyping(uint32_t friendId, bool typing)
{
    bool ret = tox_self_set_typing(tox, friendId, typing, nullptr);
    if (!ret)
        emit failedToSetTyping(typing);
}

void Core::sendGroupMessageWithType(int groupId, const QString& message, TOX_MESSAGE_TYPE type)
{
    QList<QString> cMessages = splitMessage(message, MAX_GROUP_MESSAGE_LEN);

    for (auto& part : cMessages) {
        ToxString cMsg(part);
        TOX_ERR_CONFERENCE_SEND_MESSAGE error;
        bool success =
            tox_conference_send_message(tox, groupId, type, cMsg.data(), cMsg.size(), &error);

        if (success && error == TOX_ERR_CONFERENCE_SEND_MESSAGE_OK) {
            return;
        }

        qCritical() << "Fail of tox_conference_send_message";
        switch (error) {
        case TOX_ERR_CONFERENCE_SEND_MESSAGE_CONFERENCE_NOT_FOUND:
            qCritical() << "Conference not found";
            return;
        case TOX_ERR_CONFERENCE_SEND_MESSAGE_FAIL_SEND:
            qCritical() << "Conference message failed to send";
            return;
        case TOX_ERR_CONFERENCE_SEND_MESSAGE_NO_CONNECTION:
            qCritical() << "No connection";
            return;
        case TOX_ERR_CONFERENCE_SEND_MESSAGE_TOO_LONG:
            qCritical() << "Meesage too long";
            return;
        default:
            break;
        }
        emit groupSentResult(groupId, message, -1);
    }
}

void Core::sendGroupMessage(int groupId, const QString& message)
{
    sendGroupMessageWithType(groupId, message, TOX_MESSAGE_TYPE_NORMAL);
}

void Core::sendGroupAction(int groupId, const QString& message)
{
    sendGroupMessageWithType(groupId, message, TOX_MESSAGE_TYPE_ACTION);
}

void Core::changeGroupTitle(int groupId, const QString& title)
{
    ToxString cTitle(title);
    TOX_ERR_CONFERENCE_TITLE error;
    bool success = tox_conference_set_title(tox, groupId, cTitle.data(), cTitle.size(), &error);

    if (success && error == TOX_ERR_CONFERENCE_TITLE_OK) {
        emit groupTitleChanged(groupId, getUsername(), title);
        return;
    }

    qCritical() << "Fail of tox_conference_set_title";
    switch (error) {
    case TOX_ERR_CONFERENCE_TITLE_CONFERENCE_NOT_FOUND:
        qCritical() << "Conference not found";
        break;
    case TOX_ERR_CONFERENCE_TITLE_FAIL_SEND:
        qCritical() << "Conference title failed to send";
        break;
    case TOX_ERR_CONFERENCE_TITLE_INVALID_LENGTH:
        qCritical() << "Invalid length";
        break;
    default:
        break;
    }
}

void Core::sendFile(uint32_t friendId, QString filename, QString filePath, long long filesize)
{
    CoreFile::sendFile(this, friendId, filename, filePath, filesize);
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

    if (!tox_friend_delete(tox, friendId, nullptr)) {
        emit failedToRemoveFriend(friendId);
        return;
    }

    profile.saveToxSave();
    emit friendRemoved(friendId);
}

void Core::removeGroup(int groupId, bool fake)
{
    if (!isReady() || fake)
        return;

    TOX_ERR_CONFERENCE_DELETE error;
    bool success = tox_conference_delete(tox, groupId, &error);

    if (success && error == TOX_ERR_CONFERENCE_DELETE_OK) {
        av->leaveGroupCall(groupId);
        return;
    }

    qCritical() << "Fail of tox_conference_delete";
    switch (error) {
    case TOX_ERR_CONFERENCE_DELETE_CONFERENCE_NOT_FOUND:
        qCritical() << "Conference not found";
        break;
    default:
        break;
    }
}

/**
 * @brief Returns our username, or an empty string on failure
 */
QString Core::getUsername() const
{
    QString sname;
    if (!tox)
        return sname;
    int size = tox_self_get_name_size(tox);
    uint8_t* name = new uint8_t[size];
    tox_self_get_name(tox, name);
    sname = ToxString(name, size).getQString();
    delete[] name;
    return sname;
}

void Core::setUsername(const QString& username)
{
    if (username == getUsername())
        return;

    ToxString cUsername(username);

    if (!tox_self_set_name(tox, cUsername.data(), cUsername.size(), nullptr)) {
        emit failedToSetUsername(username);
        return;
    }

    emit usernameSet(username);
    if (ready)
        profile.saveToxSave();
}

void Core::setAvatar(const QByteArray& data)
{
    if (!data.isEmpty()) {
        QPixmap pic;
        pic.loadFromData(data);
        profile.saveAvatar(data, getSelfId().getPublicKey().toString());
        emit selfAvatarChanged(pic);
    } else {
        emit selfAvatarChanged(QPixmap(":/img/contact_dark.svg"));
    }

    AvatarBroadcaster::setAvatar(data);
    AvatarBroadcaster::enableAutoBroadcast();
}

/**
 * @brief Returns our Tox ID
 */
ToxId Core::getSelfId() const
{
    uint8_t friendAddress[TOX_ADDRESS_SIZE] = {0x00};
    tox_self_get_address(tox, friendAddress);
    return ToxId(friendAddress, TOX_ADDRESS_SIZE);
}

/**
 * @brief Returns our public and private keys
 */
QPair<QByteArray, QByteArray> Core::getKeypair() const
{
    QPair<QByteArray, QByteArray> keypair;
    if (!tox)
        return keypair;

    QByteArray pk(TOX_PUBLIC_KEY_SIZE, 0x00);
    QByteArray sk(TOX_SECRET_KEY_SIZE, 0x00);

    tox_self_get_public_key(tox, reinterpret_cast<uint8_t*>(pk.data()));
    tox_self_get_secret_key(tox, reinterpret_cast<uint8_t*>(sk.data()));

    keypair.first = pk;
    keypair.second = sk;
    return keypair;
}

/**
 * @brief Returns our status message, or an empty string on failure
 */
QString Core::getStatusMessage() const
{
    QString sname;
    if (!tox)
        return sname;
    size_t size = tox_self_get_status_message_size(tox);
    uint8_t* name = new uint8_t[size];
    tox_self_get_status_message(tox, name);
    sname = ToxString(name, size).getQString();
    delete[] name;
    return sname;
}

/**
 * @brief Returns our user status
 */
Status Core::getStatus() const
{
    return (Status)tox_self_get_status(tox);
}

void Core::setStatusMessage(const QString& message)
{
    if (message == getStatusMessage())
        return;

    ToxString cMessage(message);

    if (!tox_self_set_status_message(tox, cMessage.data(), cMessage.size(), nullptr)) {
        emit failedToSetStatusMessage(message);
        return;
    }

    if (ready)
        profile.saveToxSave();
    emit statusMessageSet(message);
}

void Core::setStatus(Status status)
{
    TOX_USER_STATUS userstatus;
    switch (status) {
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
        name[name.length() - 1] = '_';

    return name;
}

/**
 * @brief Returns the unencrypted tox save data
 */
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
    if (friendCount > 0) {
        // assuming there are not that many friends to fill up the whole stack
        uint32_t* ids = new uint32_t[friendCount];
        tox_self_get_friend_list(tox, ids);
        uint8_t friendPk[TOX_PUBLIC_KEY_SIZE] = {0x00};
        for (int32_t i = 0; i < static_cast<int32_t>(friendCount); ++i) {
            if (tox_friend_get_public_key(tox, ids[i], friendPk, nullptr)) {
                emit friendAdded(ids[i], ToxPk(friendPk));

                const size_t nameSize = tox_friend_get_name_size(tox, ids[i], nullptr);
                if (nameSize && nameSize != SIZE_MAX) {
                    uint8_t* name = new uint8_t[nameSize];
                    if (tox_friend_get_name(tox, ids[i], name, nullptr))
                        emit friendUsernameChanged(ids[i], ToxString(name, nameSize).getQString());
                    delete[] name;
                }

                const size_t statusMessageSize =
                    tox_friend_get_status_message_size(tox, ids[i], nullptr);
                if (statusMessageSize != SIZE_MAX) {
                    uint8_t* statusMessage = new uint8_t[statusMessageSize];
                    if (tox_friend_get_status_message(tox, ids[i], statusMessage, nullptr)) {
                        emit friendStatusMessageChanged(ids[i], ToxString(statusMessage,
                                                                          statusMessageSize).getQString());
                    }
                    delete[] statusMessage;
                }

                checkLastOnline(ids[i]);
            }
        }
        delete[] ids;
    }
}

void Core::checkLastOnline(uint32_t friendId)
{
    const uint64_t lastOnline = tox_friend_get_last_online(tox, friendId, nullptr);
    if (lastOnline != std::numeric_limits<uint64_t>::max())
        emit friendLastSeenChanged(friendId, QDateTime::fromTime_t(lastOnline));
}

/**
 * @brief Returns the list of friendIds in our friendlist, an empty list on error
 */
QVector<uint32_t> Core::getFriendList() const
{
    QVector<uint32_t> friends;
    friends.resize(tox_self_get_friend_list_size(tox));
    tox_self_get_friend_list(tox, friends.data());
    return friends;
}

/**
 * @brief Print in console text of error.
 * @param error Error to handle.
 * @return True if no error, false otherwise.
 */
bool Core::parsePeerQueryError(TOX_ERR_CONFERENCE_PEER_QUERY error) const
{
    switch (error) {
    case TOX_ERR_CONFERENCE_PEER_QUERY_OK:
        return true;
    case TOX_ERR_CONFERENCE_PEER_QUERY_CONFERENCE_NOT_FOUND:
        qCritical() << "Conference not found";
        return false;
    case TOX_ERR_CONFERENCE_PEER_QUERY_NO_CONNECTION:
        qCritical() << "No connection";
        return false;
    case TOX_ERR_CONFERENCE_PEER_QUERY_PEER_NOT_FOUND:
        qCritical() << "Peer not found";
        return false;
    default:
        return false;
    }
}

/**
 * @brief Get number of peers in the conference.
 * @return The number of peers in the conference. UINT32_MAX on failure.
 */
uint32_t Core::getGroupNumberPeers(int groupId) const
{
    TOX_ERR_CONFERENCE_PEER_QUERY error;
    uint32_t count = tox_conference_peer_count(tox, groupId, &error);

    if (!parsePeerQueryError(error))
        return std::numeric_limits<uint32_t>::max();

    return count;
}

/**
 * @brief Get the name of a peer of a group
 */
QString Core::getGroupPeerName(int groupId, int peerId) const
{
    uint8_t nameArray[TOX_MAX_NAME_LENGTH];
    TOX_ERR_CONFERENCE_PEER_QUERY error;
    size_t length = tox_conference_peer_get_name_size(tox, groupId, peerId, &error);
    if (!parsePeerQueryError(error))
        return QString{};

    bool success = tox_conference_peer_get_name(tox, groupId, peerId, nameArray, &error);
    if (!parsePeerQueryError(error) || !success) {
        qWarning() << "getGroupPeerName: Unknown error";
        return QString{};
    }

    return ToxString(nameArray, length).getQString();
}

/**
 * @brief Get the public key of a peer of a group
 */
ToxPk Core::getGroupPeerPk(int groupId, int peerId) const
{
    uint8_t friendPk[TOX_PUBLIC_KEY_SIZE] = {0x00};
    TOX_ERR_CONFERENCE_PEER_QUERY error;
    bool success = tox_conference_peer_get_public_key(tox, groupId, peerId, friendPk, &error);
    if (!parsePeerQueryError(error) || !success) {
        qWarning() << "getGroupPeerToxId: Unknown error";
        return ToxPk();
    }

    return ToxPk(friendPk);
}

/**
 * @brief Get the names of the peers of a group
 */
QList<QString> Core::getGroupPeerNames(int groupId) const
{
    if (!tox) {
        qWarning() << "Can't get group peer names, tox is null";
        return {};
    }

    uint32_t nPeers = getGroupNumberPeers(groupId);
    if (nPeers == std::numeric_limits<uint32_t>::max()) {
        qWarning() << "getGroupPeerNames: Unable to get number of peers";
        return {};
    }

    // TODO: Change to std::vector
    std::unique_ptr<uint8_t[][TOX_MAX_NAME_LENGTH]> namesArray{new uint8_t[nPeers][TOX_MAX_NAME_LENGTH]};

    std::unique_ptr<uint16_t[]> lengths{new uint16_t[nPeers]};
    TOX_ERR_CONFERENCE_PEER_QUERY error;

    uint32_t count = tox_conference_peer_count(tox, groupId, &error);
    if (!parsePeerQueryError(error)) {
        return {};
    }

    if (count != nPeers) {
        qWarning() << "getGroupPeerNames: Unexpected peer count";
        return {};
    }

    QList<QString> names;
    for (uint32_t i = 0; i < nPeers; ++i) {
        lengths[i] = tox_conference_peer_get_name_size(tox, groupId, i, &error);
        bool ok = tox_conference_peer_get_name(tox, groupId, i, namesArray[i], &error);
        if (parsePeerQueryError(error) && ok) {
            names.push_back(ToxString(namesArray[i], lengths[i]).getQString());
        }
    }

    return names;
}

/**
 * @brief Print in console text of error.
 * @param error Error to handle.
 * @return True if no error, false otherwise.
 */
bool Core::parseConferenceJoinError(TOX_ERR_CONFERENCE_JOIN error) const
{
    switch (error) {
    case TOX_ERR_CONFERENCE_JOIN_OK:
        return true;
    case TOX_ERR_CONFERENCE_JOIN_DUPLICATE:
        qCritical() << "Conference duplicate";
        return false;
    case TOX_ERR_CONFERENCE_JOIN_FAIL_SEND:
        qCritical() << "Conference join failed to send";
        return false;
    case TOX_ERR_CONFERENCE_JOIN_FRIEND_NOT_FOUND:
        qCritical() << "Friend not found";
        return false;
    case TOX_ERR_CONFERENCE_JOIN_INIT_FAIL:
        qCritical() << "Init fail";
        return false;
    case TOX_ERR_CONFERENCE_JOIN_INVALID_LENGTH:
        qCritical() << "Invalid length";
        return false;
    case TOX_ERR_CONFERENCE_JOIN_WRONG_TYPE:
        qCritical() << "Wrong conference type";
        return false;
    default:
        return false;
    }
}

/**
 * @brief Accept a groupchat invite.
 * @param friendnumber Id of friend in friend list.
 * @param type Chat type (TEXT or AV).
 * @param friend_group_public_key Received via the `conference_invite` event.
 * @param length The size of @friend_group_public_key.
 *
 * @return Conference number on success, UINT32_MAX on failure.
 */
uint32_t Core::joinGroupchat(int32_t friendnumber, uint8_t type,
                             const uint8_t* friend_group_public_key, uint16_t length) const
{
    if (type == TOX_CONFERENCE_TYPE_TEXT) {
        qDebug() << QString("Trying to join text groupchat invite sent by friend %1").arg(friendnumber);
        TOX_ERR_CONFERENCE_JOIN error;
        uint32_t groupId =
            tox_conference_join(tox, friendnumber, friend_group_public_key, length, &error);
        if (parseConferenceJoinError(error))
            return groupId;
        else
            return std::numeric_limits<uint32_t>::max();
    } else if (type == TOX_CONFERENCE_TYPE_AV) {
        qDebug() << QString("Trying to join AV groupchat invite sent by friend %1").arg(friendnumber);
        return toxav_join_av_groupchat(tox, friendnumber, friend_group_public_key, length,
                                       CoreAV::groupCallCallback, const_cast<Core*>(this));
    } else {
        qWarning() << "joinGroupchat: Unknown groupchat type " << type;
        return std::numeric_limits<uint32_t>::max();
    }
}

/**
 * @brief Quit a groupchat
 */
void Core::quitGroupChat(int groupId) const
{
    TOX_ERR_CONFERENCE_DELETE error;
    tox_conference_delete(tox, groupId, &error);

    switch (error) {
    case TOX_ERR_CONFERENCE_DELETE_OK:
        return;
    case TOX_ERR_CONFERENCE_DELETE_CONFERENCE_NOT_FOUND:
        qCritical() << "Conference not found";
        return;
    default:
        return;
    }
}

void Core::groupInviteFriend(uint32_t friendId, int groupId)
{
    TOX_ERR_CONFERENCE_INVITE error;
    tox_conference_invite(tox, friendId, groupId, &error);

    switch (error) {
    case TOX_ERR_CONFERENCE_INVITE_OK:
        break;
    case TOX_ERR_CONFERENCE_INVITE_CONFERENCE_NOT_FOUND:
        qCritical() << "Conference not found";
        break;
    case TOX_ERR_CONFERENCE_INVITE_FAIL_SEND:
        qCritical() << "Conference invite failed to send";
        break;
    default:
        break;
    }
}

int Core::createGroup(uint8_t type)
{
    if (type == TOX_CONFERENCE_TYPE_TEXT) {
        TOX_ERR_CONFERENCE_NEW error;
        uint32_t groupId = tox_conference_new(tox, &error);

        switch (error) {
        case TOX_ERR_CONFERENCE_NEW_OK:
            emit emptyGroupCreated(groupId);
            return groupId;
        case TOX_ERR_CONFERENCE_NEW_INIT:
            qCritical() << "The conference instance failed to initialize";
            return std::numeric_limits<uint32_t>::max();
        default:
            return std::numeric_limits<uint32_t>::max();
        }
    } else if (type == TOX_CONFERENCE_TYPE_AV) {
        uint32_t groupId = toxav_add_av_groupchat(tox, CoreAV::groupCallCallback, this);
        emit emptyGroupCreated(groupId);
        return groupId;
    } else {
        qWarning() << "createGroup: Unknown type " << type;
        return -1;
    }
}

/**
 * @brief Checks if a friend is online. Unknown friends are considered offline.
 */
bool Core::isFriendOnline(uint32_t friendId) const
{
    TOX_CONNECTION connetion = tox_friend_get_connection_status(tox, friendId, nullptr);
    return connetion != TOX_CONNECTION_NONE;
}

/**
 * @brief Checks if we have a friend by public key
 */
bool Core::hasFriendWithPublicKey(const ToxPk& publicKey) const
{
    // Validity check
    if (publicKey.isEmpty()) {
        return false;
    }

    // TODO: error handling
    uint32_t friendId = tox_friend_by_public_key(tox, publicKey.getBytes(), nullptr);

    return friendId != std::numeric_limits<uint32_t>::max();
}

/**
 * @brief Get the public key part of the ToxID only
 */
ToxPk Core::getFriendPublicKey(uint32_t friendNumber) const
{
    uint8_t rawid[TOX_PUBLIC_KEY_SIZE];
    if (!tox_friend_get_public_key(tox, friendNumber, rawid, nullptr)) {
        qWarning() << "getFriendPublicKey: Getting public key failed";
        return ToxPk();
    }

    return ToxPk(rawid);
}

/**
 * @brief Get the username of a friend
 */
QString Core::getFriendUsername(uint32_t friendnumber) const
{
    size_t namesize = tox_friend_get_name_size(tox, friendnumber, nullptr);
    if (namesize == SIZE_MAX) {
        qWarning() << "getFriendUsername: Failed to get name size for friend " << friendnumber;
        return QString();
    }
    uint8_t* name = new uint8_t[namesize];
    tox_friend_get_name(tox, friendnumber, name, nullptr);
    ToxString sname(name, namesize);
    delete[] name;
    return sname.getQString();
}

QList<QString> Core::splitMessage(const QString& message, int maxLen)
{
    QList<QString> splittedMsgs;
    QByteArray ba_message(message.toUtf8());

    while (ba_message.size() > maxLen) {
        int splitPos = ba_message.lastIndexOf(' ', maxLen - 1);
        if (splitPos <= 0) {
            splitPos = maxLen;
            if (ba_message[splitPos] & 0x80) {
                do {
                    splitPos--;
                } while (!(ba_message[splitPos] & 0x40));
            }
            splitPos--;
        }

        splittedMsgs.push_back(QString(ba_message.left(splitPos + 1)));
        ba_message = ba_message.mid(splitPos + 1);
    }

    splittedMsgs.push_back(QString(ba_message));

    return splittedMsgs;
}

QString Core::getPeerName(const ToxPk& id) const
{
    QString name;
    uint32_t friendId = tox_friend_by_public_key(tox, id.getBytes(), nullptr);
    if (friendId == std::numeric_limits<uint32_t>::max()) {
        qWarning() << "getPeerName: No such peer";
        return name;
    }

    const size_t nameSize = tox_friend_get_name_size(tox, friendId, nullptr);
    if (nameSize == SIZE_MAX)
        return name;

    uint8_t* cname = new uint8_t[nameSize < TOX_MAX_NAME_LENGTH ? TOX_MAX_NAME_LENGTH : nameSize];
    if (!tox_friend_get_name(tox, friendId, cname, nullptr)) {
        qWarning() << "getPeerName: Can't get name of friend " + QString().setNum(friendId);
        delete[] cname;
        return name;
    }

    name = ToxString(cname, nameSize).getQString();
    delete[] cname;
    return name;
}

/**
 * @brief Most of the API shouldn't be used until Core is ready, call start() first
 */
bool Core::isReady() const
{
    return av && av->getToxAv() && tox && ready;
}


/**
 * @brief Sets the NoSpam value to prevent friend request spam
 * @param nospam an arbitrary which becomes part of the Tox ID
 */
void Core::setNospam(uint32_t nospam)
{
    tox_self_set_nospam(tox, nospam);

    emit idSet(getSelfId());
}

/**
 * @brief Returns the unencrypted tox save data
 */
void Core::killTimers(bool onlyStop)
{
    assert(QThread::currentThread() == coreThread);
    av->stop();
    toxTimer->stop();
    if (!onlyStop) {
        delete toxTimer;
        toxTimer = nullptr;
    }
}

/**
 * @brief Reinitialized the core.
 * @warning Must be called from the Core thread, with the GUI thread ready to process events.
 */
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
