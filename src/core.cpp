/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>

    This file is part of Tox Qt GUI.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "core.h"
#include "nexus.h"
#include "misc/cdata.h"
#include "misc/cstring.h"
#include "misc/settings.h"
#include "widget/gui.h"
#include "historykeeper.h"
#include "src/audio.h"

#include <tox/tox.h>

#include <ctime>
#include <functional>
#include <cassert>

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
QList<ToxFile> Core::fileSendQueue;
QList<ToxFile> Core::fileRecvQueue;
QHash<int, ToxGroupCall> Core::groupCalls;
QThread* Core::coreThread{nullptr};

#define MAX_GROUP_MESSAGE_LEN 1024

Core::Core(Camera* cam, QThread *CoreThread, QString loadPath) :
    tox(nullptr), camera(cam), loadPath(loadPath), ready{false}
{
    qDebug() << "Core: loading Tox from" << loadPath;

    coreThread = CoreThread;

    Audio::getInstance();

    videobuf = new uint8_t[videobufsize];

    for (int i = 0; i < ptCounter; i++)
        pwsaltedkeys[i] = nullptr;

    toxTimer = new QTimer(this);
    toxTimer->setSingleShot(true);
    connect(toxTimer, &QTimer::timeout, this, &Core::process);
    //connect(fileTimer, &QTimer::timeout, this, &Core::fileHeartbeat);
    connect(&Settings::getInstance(), &Settings::dhtServerListChanged, this, &Core::process);
    connect(this, SIGNAL(fileTransferFinished(ToxFile)), this, SLOT(onFileTransferFinished(ToxFile)));

    for (int i=0; i<TOXAV_MAX_CALLS;i++)
    {
        calls[i].active = false;
        calls[i].alSource = 0;
        calls[i].sendAudioTimer = new QTimer();
        calls[i].sendVideoTimer = new QTimer();
        calls[i].sendAudioTimer->moveToThread(coreThread);
        calls[i].sendVideoTimer->moveToThread(coreThread);
        connect(calls[i].sendVideoTimer, &QTimer::timeout, [this,i](){sendCallVideo(i);});
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

    saveConfiguration();
    toxTimer->stop();
    coreThread->exit(0);
    while (coreThread->isRunning())
    {
        qApp->processEvents();
        coreThread->wait(500);
    }

    deadifyTox();

    if (videobuf)
    {
        delete[] videobuf;
        videobuf=nullptr;
    }

    Audio::closeInput();
    Audio::closeOutput();
}

Core* Core::getInstance()
{
    return Nexus::getCore();
}

void Core::make_tox(QByteArray savedata)
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

    if (proxyType != ProxyType::ptNone)
    {
        QString proxyAddr = Settings::getInstance().getProxyAddr();
        int proxyPort = Settings::getInstance().getProxyPort();

        if (proxyAddr.length() > 255)
        {
            qWarning() << "Core: proxy address" << proxyAddr << "is too long";
        }
        else if (proxyAddr != "" && proxyPort > 0)
        {
            qDebug() << "Core: using proxy" << proxyAddr << ":" << proxyPort;
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

    tox = tox_new(&toxOptions, (uint8_t*)savedata.data(), savedata.size(), nullptr);
    if (tox == nullptr)
    {
        if (enableIPv6) // Fallback to IPv4
        {
            toxOptions.ipv6_enabled = false;
            tox = tox_new(&toxOptions, (uint8_t*)savedata.data(), savedata.size(), nullptr);
            if (tox == nullptr)
            {
                if (toxOptions.proxy_type != TOX_PROXY_TYPE_NONE)
                {
                    qCritical() << "Core: bad proxy! no toxcore!";
                    emit badProxy();
                }
                else
                {
                    qCritical() << "Tox core failed to start";
                    emit failedToStart();
                }
                return;
            }
            else
                qWarning() << "Core failed to start with IPv6, falling back to IPv4. LAN discovery may not work properly.";
        }
        else if (toxOptions.proxy_type != TOX_PROXY_TYPE_NONE)
        {
            emit badProxy();
            return;
        }
        else
        {
            qCritical() << "Tox core failed to start";
            emit failedToStart();
            return;
        }
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
    qDebug() << "Core: Starting up";

    QByteArray savedata = loadToxSave(loadPath);

    make_tox(savedata);

    qsrand(time(nullptr));

    /** TODO: Review this mess. Actually rewrite it all
     * I have no idea what the whole fooling around with loadPath bussiness means anymore.
     * Let's write something clear that doesn't rely on magic global state instead
     * We need to: 1) Find a qTox profile, create if needed 2) Find a tox save, decrypt/create if needed
     * Not sure about the order tho. Look into this.
    if (true)
    {
        if (loadPath.isEmpty())
        {
            QString profile;
            if ((profile = loadOldInformation()).isEmpty())
            {
                qCritical() << "Core: loadConfiguration failed, exiting now";
                emit failedToStart();
                return;
            }
            else
            {
                loadPath = QDir(Settings::getSettingsDirPath()).filePath(profile + TOX_EXT);
                Settings::getInstance().switchProfile(profile);
                HistoryKeeper::resetInstance(); // I'm not actually sure if this is necessary
            }
        }
        // loadPath is meaningless after this
        loadPath = "";
    }
    else // new ID
    {
        QString id = getSelfId().toString();
        if (!id.isEmpty())
            emit idSet(id);
        setStatusMessage(tr("Toxing on qTox")); // this also solves the not updating issue
        setUsername(tr("qTox User"));
    }
    */

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
    if (Settings::getInstance().getEnableLogging() && Settings::getInstance().getEncryptLogs())
        checkEncryptedHistory();

    loadFriends();

    tox_callback_friend_request(tox, onFriendRequest, this);
    tox_callback_friend_message(tox, onFriendMessage, this);
    tox_callback_friend_name(tox, onFriendNameChange, this);
    tox_callback_friend_typing(tox, onFriendTypingChange, this);
    tox_callback_friend_status_message(tox, onStatusMessageChanged, this);
    tox_callback_friend_status(tox, onUserStatusChanged, this);
    tox_callback_friend_connection_status(tox, onConnectionStatusChanged, this);
    tox_callback_group_invite(tox, onGroupInvite, this);
    tox_callback_group_message(tox, onGroupMessage, this);
    tox_callback_group_namelist_change(tox, onGroupNamelistChange, this);
    tox_callback_group_title(tox, onGroupTitleChange, this);
    tox_callback_group_action(tox, onGroupAction, this);
    //tox_callback_file_send_request(tox, onFileSendRequestCallback, this);
    //tox_callback_file_control(tox, onFileControlCallback, this);
    //tox_callback_file_data(tox, onFileDataCallback, this);
    //tox_callback_avatar_info(tox, onAvatarInfoCallback, this);
    //tox_callback_avatar_data(tox, onAvatarDataCallback, this);
    tox_callback_friend_read_receipt(tox, onReadReceiptCallback, this);

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
        qDebug() << "Core: Error loading self avatar";

    ready = true;

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
        tolerance = CORE_DISCONNECT_TOLERANCE;
    else if (!(--tolerance))
    {
        bootstrapDht();
        tolerance = 3*CORE_DISCONNECT_TOLERANCE;
    }

    toxTimer->start(qMin(tox_iteration_interval(tox), toxav_do_interval(toxav)));
}

bool Core::checkConnection()
{
    static bool isConnected = false;
    //static int count = 0;
    bool toxConnected = tox_self_get_connection_status(tox) != TOX_CONNECTION_NONE;

    if (toxConnected && !isConnected) {
        qDebug() << "Core: Connected to DHT";
        emit connected();
        isConnected = true;
        //if (count) qDebug() << "Core: disconnect count:" << count;
        //count = 0;
    } else if (!toxConnected && isConnected) {
        qDebug() << "Core: Disconnected to DHT";
        emit disconnected();
        isConnected = false;
        //count++;
    } //else if (!toxConnected) count++;
    return isConnected;
}

void Core::bootstrapDht()
{
    const Settings& s = Settings::getInstance();
    QList<Settings::DhtServer> dhtServerList = s.getDhtServerList();

    int listSize = dhtServerList.size();
    if (listSize == 0)
    {
        qDebug() << "Settings: no bootstrap list?!?";
        return;
    }
    static int j = qrand() % listSize;

    qDebug() << "Core: Bootstrapping to the DHT ...";

    int i=0;
    while (i < 2) // i think the more we bootstrap, the more we jitter because the more we overwrite nodes
    {
        const Settings::DhtServer& dhtServer = dhtServerList[j % listSize];
        if (tox_bootstrap(tox, dhtServer.address.toLatin1().data(),
            dhtServer.port, CUserId(dhtServer.userId).data(), nullptr) == 1)
            qDebug() << QString("Core: Bootstrapping from ")+dhtServer.name+QString(", addr ")+dhtServer.address.toLatin1().data()
                        +QString(", port ")+QString().setNum(dhtServer.port);
        else
            qDebug() << "Core: Error bootstrapping from "+dhtServer.name;

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
    /// TODO: Emit action or message depending on type
    emit static_cast<Core*>(core)->friendMessageReceived(friendId,CString::toString(cMessage, cMessageSize), false);
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

void Core::onConnectionStatusChanged(Tox*/* tox*/, uint32_t friendId, TOX_CONNECTION status, void* core)
{
    Status friendStatus = status ? Status::Online : Status::Offline;
    emit static_cast<Core*>(core)->friendStatusChanged(friendId, friendStatus);
    if (friendStatus == Status::Offline) {
        static_cast<Core*>(core)->checkLastOnline(friendId);

        for (ToxFile& f : fileSendQueue)
        {
            if (f.friendId == friendId && f.status == ToxFile::TRANSMITTING)
            {
                f.status = ToxFile::BROKEN;
                emit static_cast<Core*>(core)->fileTransferBrokenUnbroken(f, true);
            }
        }
        for (ToxFile& f : fileRecvQueue)
        {
            if (f.friendId == friendId && f.status == ToxFile::TRANSMITTING)
            {
                f.status = ToxFile::BROKEN;
                emit static_cast<Core*>(core)->fileTransferBrokenUnbroken(f, true);
            }
        }
    } else {
        for (ToxFile& f : fileRecvQueue)
        {
            if (f.friendId == friendId && f.status == ToxFile::BROKEN)
            {
                qDebug() << QString("Core::onConnectionStatusChanged: %1: resuming broken filetransfer from position: %2").arg(f.file->fileName()).arg(f.bytesSent);
                tox_file_control(static_cast<Core*>(core)->tox, friendId, f.fileNum, TOX_FILE_CONTROL_RESUME, nullptr);
                emit static_cast<Core*>(core)->fileTransferBrokenUnbroken(f, false);
            }
        }
    }
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
        qDebug() << QString("Core: Text group invite by %1").arg(friendNumber);
        emit static_cast<Core*>(core)->groupInviteReceived(friendNumber,type,pk);
    }
    else if (type == TOX_GROUPCHAT_TYPE_AV)
    {
        qDebug() << QString("Core: AV group invite by %1").arg(friendNumber);
        emit static_cast<Core*>(core)->groupInviteReceived(friendNumber,type,pk);
    }
    else
    {
        qWarning() << "Core: Group invite with unknown type "<<type;
    }
}

void Core::onGroupMessage(Tox*, int groupnumber, int peernumber, const uint8_t * message, uint16_t length, void *_core)
{
    Core* core = static_cast<Core*>(_core);
    emit core->groupMessageReceived(groupnumber, peernumber, CString::toString(message, length), false);
}

void Core::onGroupNamelistChange(Tox*, int groupnumber, int peernumber, uint8_t change, void *core)
{
    qDebug() << QString("Core: Group namelist change %1:%2 %3").arg(groupnumber).arg(peernumber).arg(change);
    emit static_cast<Core*>(core)->groupNamelistChanged(groupnumber, peernumber, change);
}

void Core::onGroupTitleChange(Tox*, int groupnumber, int peernumber, const uint8_t* title, uint8_t len, void* _core)
{
    qDebug() << "Core: group" << groupnumber << "title changed by" << peernumber;
    Core* core = static_cast<Core*>(_core);
    QString author;
    if (peernumber >= 0)
        author = core->getGroupPeerName(groupnumber, peernumber);
    emit core->groupTitleChanged(groupnumber, author, CString::toString(title, len));
}

void Core::onFileSendRequestCallback(Tox*, uint32_t friendnumber, uint8_t filenumber, uint64_t filesize,
                                          const uint8_t *filename, uint16_t filename_length, void *core)
{
    qDebug() << QString("Core: Received file request %1 with friend %2").arg(filenumber).arg(friendnumber);

    ToxFile file{filenumber, friendnumber,
                CString::toString(filename,filename_length).toUtf8(), "", ToxFile::RECEIVING};
    file.filesize = filesize;
    fileRecvQueue.append(file);
    emit static_cast<Core*>(core)->fileReceiveRequested(fileRecvQueue.last());
}
void Core::onFileControlCallback(Tox* tox, uint32_t friendnumber, uint8_t receive_send, uint8_t filenumber,
                                      uint8_t control_type, const uint8_t* data, uint16_t length, void *core)
{
    ToxFile* file{nullptr};
    if (receive_send == 1)
    {
        for (ToxFile& f : fileSendQueue)
        {
            if (f.fileNum == filenumber && f.friendId == friendnumber)
            {
                file = &f;
                break;
            }
        }
    }
    else
    {
        for (ToxFile& f : fileRecvQueue)
        {
            if (f.fileNum == filenumber && f.friendId == friendnumber)
            {
                file = &f;
                break;
            }
        }
    }
    if (!file)
    {
        qWarning("Core::onFileControlCallback: No such file in queue");
        return;
    }
    if      (receive_send == 1 && control_type == TOX_FILE_CONTROL_RESUME)
    {
        file->status = ToxFile::TRANSMITTING;
        emit static_cast<Core*>(core)->fileTransferAccepted(*file);
        qDebug() << "Core: File control callback, file accepted";
        file->sendTimer = new QTimer(static_cast<Core*>(core));
        connect(file->sendTimer, &QTimer::timeout, std::bind(sendAllFileData,static_cast<Core*>(core), file));
        file->sendTimer->setSingleShot(true);
        file->sendTimer->start(TOX_FILE_INTERVAL);
    }
    else if (receive_send == 1 && control_type == TOX_FILE_CONTROL_CANCEL)
    {
        qDebug() << QString("Core::onFileControlCallback: Transfer of file %1 cancelled by friend %2")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferCancelled(*file);
        // Wait for sendAllFileData to return before deleting the ToxFile, we MUST ensure this or we'll use after free
        if (file->sendTimer)
        {
            QThread::msleep(1);
            qApp->processEvents();
            if (file->sendTimer)
            {
                delete file->sendTimer;
                file->sendTimer = nullptr;
            }
        }
        removeFileFromQueue((bool)receive_send, file->friendId, file->fileNum);
    }
    else if (receive_send == 1 && false /* && control_type == TOX_FILECONTROL_FINISHED*/ ) ///TODO: FIXME: Detect finished transfers
    {
        qDebug() << QString("Core::onFileControlCallback: Transfer of file %1 to friend %2 is complete")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferFinished(*file);
        removeFileFromQueue((bool)receive_send, file->friendId, file->fileNum);
    }
    else if (receive_send == 0 && control_type == TOX_FILE_CONTROL_CANCEL)
    {
        qDebug() << QString("Core::onFileControlCallback: Transfer of file %1 cancelled by friend %2")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferCancelled(*file);
        removeFileFromQueue((bool)receive_send, file->friendId, file->fileNum);
    }
    else if (receive_send == 0 && false /* && control_type == TOX_FILECONTROL_FINISHED*/ ) ///TODO: FIXME: Detect finished transfers
    {
        qDebug() << QString("Core::onFileControlCallback: Reception of file %1 from %2 finished")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferFinished(*file);
        // confirm receive is complete
        ///tox_file_control(tox, file->friendId, 1, file->fileNum, TOX_FILECONTROL_FINISHED, nullptr, 0);
        removeFileFromQueue((bool)receive_send, file->friendId, file->fileNum);
    }
    else if (receive_send == 0 && control_type == TOX_FILE_CONTROL_RESUME)
    {
        if (file->status == ToxFile::BROKEN)
        {
            emit static_cast<Core*>(core)->fileTransferBrokenUnbroken(*file, false);
            file->status = ToxFile::TRANSMITTING;
        }
        emit static_cast<Core*>(core)->fileTransferRemotePausedUnpaused(*file, false);
    }
    else if ((receive_send == 0 || receive_send == 1) && control_type == TOX_FILE_CONTROL_PAUSE)
    {
        emit static_cast<Core*>(core)->fileTransferRemotePausedUnpaused(*file, true);
    }
    else if (receive_send == 1 && control_type == TOX_FILE_CONTROL_RESUME)
    {
        if (length != sizeof(uint64_t))
            return;

        qDebug() << "Core::onFileControlCallback: TOX_FILECONTROL_RESUME_BROKEN";

        uint64_t resumePos = *reinterpret_cast<const uint64_t*>(data);

        if (resumePos >= (unsigned)file->filesize)
        {
            qWarning() << "Core::onFileControlCallback: invalid resume position";
            tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr); // don't sure about it
            return;
        }

        file->status = ToxFile::TRANSMITTING;
        emit static_cast<Core*>(core)->fileTransferBrokenUnbroken(*file, false);

        file->bytesSent = resumePos;
        tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_RESUME, nullptr);
    }
    else
    {
        qDebug() << QString("Core: File control callback, receive_send=%1, control_type=%2")
                    .arg(receive_send).arg(control_type);
    }
}

void Core::onFileDataCallback(Tox*, uint32_t friendnumber, uint8_t filenumber, const uint8_t *data, uint16_t length, void *core)
{
    ToxFile* file{nullptr};
    for (ToxFile& f : fileRecvQueue)
    {
        if (f.fileNum == filenumber && f.friendId == friendnumber)
        {
            file = &f;
            break;
        }
    }
    if (!file)
    {
        qWarning("Core::onFileDataCallback: No such file in queue");
        return;
    }

    file->file->write((char*)data,length);
    file->bytesSent += length;
    //qDebug() << QString("Core::onFileDataCallback: received %1/%2 bytes").arg(file->bytesSent).arg(file->filesize);
    emit static_cast<Core*>(core)->fileTransferInfo(*file);
}

void Core::onAvatarInfoCallback(Tox*, uint32_t friendnumber, uint8_t format,
                                uint8_t* hash, void* _core)
{
    assert(0);
    /* TODO: Address this avatar function
    Core* core = static_cast<Core*>(_core);

    if (format == TOX_AVATAR_FORMAT_NONE)
    {
        //qDebug() << "Core: Got null avatar info from" << core->getFriendUsername(friendnumber);
        emit core->friendAvatarRemoved(friendnumber);
        QFile::remove(QDir(Settings::getSettingsDirPath()).filePath("avatars/"+core->getFriendAddress(friendnumber).left(64)+".png"));
        QFile::remove(QDir(Settings::getSettingsDirPath()).filePath("avatars/"+core->getFriendAddress(friendnumber).left(64)+".hash"));
    }
    else
    {
        QByteArray oldHash = Settings::getInstance().getAvatarHash(core->getFriendAddress(friendnumber));
        if (QByteArray((char*)hash, TOX_HASH_LENGTH) != oldHash) 
        // comparison failed miserably if I didn't convert hash to QByteArray
        {
            qDebug() << "Core: Got new avatar info from" << core->getFriendUsername(friendnumber);
            tox_request_avatar_data(core->tox, friendnumber);
        }
        //else
        //    qDebug() << "Core: Got same avatar info from" << core->getFriendUsername(friendnumber);
    }
    */
}

void Core::onAvatarDataCallback(Tox*, uint32_t friendnumber, uint8_t,
                        uint8_t *hash, uint8_t *data, uint32_t datalen, void *core)
{
    QPixmap pic;
    pic.loadFromData((uchar*)data, datalen);
    if (!pic.isNull())
    {
        qDebug() << "Core: Got avatar data from" << static_cast<Core*>(core)->getFriendUsername(friendnumber);
        Settings::getInstance().saveAvatar(pic, static_cast<Core*>(core)->getFriendAddress(friendnumber));
        Settings::getInstance().saveAvatarHash(QByteArray((char*)hash, TOX_HASH_LENGTH), static_cast<Core*>(core)->getFriendAddress(friendnumber));
        emit static_cast<Core*>(core)->friendAvatarChanged(friendnumber, pic);
    }
}

void Core::onReadReceiptCallback(Tox*, uint32_t friendnumber, uint32_t receipt, void *core)
{
     emit static_cast<Core*>(core)->receiptRecieved(friendnumber, receipt);
}

void Core::acceptFriendRequest(const QString& userId)
{
    uint32_t friendId = tox_friend_add_norequest(tox, CUserId(userId).data(), nullptr);
    if (friendId == UINT32_MAX) {
        emit failedToAddFriend(userId);
    } else {
        saveConfiguration();
        emit friendAdded(friendId, userId);
    }
}

void Core::requestFriendship(const QString& friendAddress, const QString& message)
{
    const QString userId = friendAddress.mid(0, TOX_PUBLIC_KEY_SIZE * 2);

    if (hasFriendWithAddress(friendAddress))
    {
        emit failedToAddFriend(userId, QString(tr("Friend is already added")));
    }
    else
    {
        qDebug() << "Core: requesting friendship of "+friendAddress;
        CString cMessage(message);

        uint32_t friendId = tox_friend_add(tox, CFriendAddress(friendAddress).data(),
                                      cMessage.data(), cMessage.size(), nullptr);
        if (friendId == UINT32_MAX)
        {
            emit failedToAddFriend(userId);
        }
        else
        {
            // Update our friendAddresses
            Settings::getInstance().updateFriendAdress(friendAddress);
            QString inviteStr = tr("/me offers friendship.");
            if (message.length())
                inviteStr = tr("/me offers friendship, \"%1\"").arg(message);
            HistoryKeeper::getInstance()->addChatEntry(userId, inviteStr, getSelfId().publicKey, QDateTime::currentDateTime(), true);
            emit friendAdded(friendId, userId);
        }
    }
    saveConfiguration();
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
    QMutexLocker mlocker(&fileSendMutex);

    QByteArray fileName = Filename.toUtf8();
    uint32_t fileNum = tox_file_send(tox, friendId, TOX_FILE_KIND_DATA, filesize, nullptr,
                                (uint8_t*)fileName.data(), fileName.size(), nullptr);
    if (fileNum == UINT32_MAX)
    {
        qWarning() << "Core::sendFile: Can't create the Tox file sender";
        emit fileSendFailed(friendId, Filename);
        return;
    }
    qDebug() << QString("Core::sendFile: Created file sender %1 with friend %2").arg(fileNum).arg(friendId);

    ToxFile file{fileNum, friendId, fileName, FilePath, ToxFile::SENDING};
    file.filesize = filesize;
    if (!file.open(false))
    {
        qWarning() << QString("Core::sendFile: Can't open file, error: %1").arg(file.file->errorString());
    }
    fileSendQueue.append(file);

    emit fileSendStarted(fileSendQueue.last());
}

void Core::pauseResumeFileSend(uint32_t friendId, uint32_t fileNum)
{
    ToxFile* file{nullptr};
    for (ToxFile& f : fileSendQueue)
    {
        if (f.fileNum == fileNum && f.friendId == friendId)
        {
            file = &f;
            break;
        }
    }
    if (!file)
    {
        qWarning("Core::pauseResumeFileSend: No such file in queue");
        return;
    }
    if (file->status == ToxFile::TRANSMITTING)
    {
        file->status = ToxFile::PAUSED;
        emit fileTransferPaused(*file);
        tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_PAUSE, nullptr);
    }
    else if (file->status == ToxFile::PAUSED)
    {
        file->status = ToxFile::TRANSMITTING;
        emit fileTransferAccepted(*file);
        tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_RESUME, nullptr);
    }
    else
        qWarning() << "Core::pauseResumeFileSend: File is stopped";
}

void Core::pauseResumeFileRecv(uint32_t friendId, uint32_t fileNum)
{
    ToxFile* file{nullptr};
    for (ToxFile& f : fileRecvQueue)
    {
        if (f.fileNum == fileNum && f.friendId == friendId)
        {
            file = &f;
            break;
        }
    }
    if (!file)
    {
        qWarning("Core::cancelFileRecv: No such file in queue");
        return;
    }
    if (file->status == ToxFile::TRANSMITTING)
    {
        file->status = ToxFile::PAUSED;
        emit fileTransferPaused(*file);
        tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_PAUSE, nullptr);
    }
    else if (file->status == ToxFile::PAUSED)
    {
        file->status = ToxFile::TRANSMITTING;
        emit fileTransferAccepted(*file);
        tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_RESUME, nullptr);
    }
    else
        qWarning() << "Core::pauseResumeFileRecv: File is stopped or broken";
}

void Core::cancelFileSend(uint32_t friendId, uint32_t fileNum)
{
    ToxFile* file{nullptr};
    for (ToxFile& f : fileSendQueue)
    {
        if (f.fileNum == fileNum && f.friendId == friendId)
        {
            file = &f;
            break;
        }
    }
    if (!file)
    {
        qWarning("Core::cancelFileSend: No such file in queue");
        return;
    }
    file->status = ToxFile::STOPPED;
    emit fileTransferCancelled(*file);
    tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr);
    while (file->sendTimer) QThread::msleep(1); // Wait until sendAllFileData returns before deleting
    removeFileFromQueue(true, friendId, fileNum);
}

void Core::cancelFileRecv(uint32_t friendId, uint32_t fileNum)
{
    ToxFile* file{nullptr};
    for (ToxFile& f : fileRecvQueue)
    {
        if (f.fileNum == fileNum && f.friendId == friendId)
        {
            file = &f;
            break;
        }
    }
    if (!file)
    {
        qWarning("Core::cancelFileRecv: No such file in queue");
        return;
    }
    file->status = ToxFile::STOPPED;
    emit fileTransferCancelled(*file);
    tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr);
    removeFileFromQueue(true, friendId, fileNum);
}

void Core::rejectFileRecvRequest(uint32_t friendId, uint32_t fileNum)
{
    ToxFile* file{nullptr};
    for (ToxFile& f : fileRecvQueue)
    {
        if (f.fileNum == fileNum && f.friendId == friendId)
        {
            file = &f;
            break;
        }
    }
    if (!file)
    {
        qWarning("Core::rejectFileRecvRequest: No such file in queue");
        return;
    }
    file->status = ToxFile::STOPPED;
    emit fileTransferCancelled(*file);
    tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr);
    removeFileFromQueue(false, friendId, fileNum);
}

void Core::acceptFileRecvRequest(uint32_t friendId, uint32_t fileNum, QString path)
{
    ToxFile* file{nullptr};
    for (ToxFile& f : fileRecvQueue)
    {
        if (f.fileNum == fileNum && f.friendId == friendId)
        {
            file = &f;
            break;
        }
    }
    if (!file)
    {
        qWarning("Core::acceptFileRecvRequest: No such file in queue");
        return;
    }
    file->setFilePath(path);
    if (!file->open(true))
    {
        qWarning() << "Core::acceptFileRecvRequest: Unable to open file";
        return;
    }
    file->status = ToxFile::TRANSMITTING;
    emit fileTransferAccepted(*file);
    tox_file_control(tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_RESUME, nullptr);
}

void Core::removeFriend(uint32_t friendId, bool fake)
{
    if (!isReady() || fake)
        return;
    if (tox_friend_delete(tox, friendId, nullptr) == false) {
        emit failedToRemoveFriend(friendId);
    } else {
        saveConfiguration();
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

    if (tox_self_set_name(tox, cUsername.data(), cUsername.size(), nullptr) == false) {
        emit failedToSetUsername(username);
    } else {
        emit usernameSet(username);
        saveConfiguration();
    }
}

void Core::setAvatar(const QByteArray& data)
{
    /// TODO: Review this function, toxcore doesn't handle avatars anymore apparently. Good.

    QPixmap pic;
    pic.loadFromData(data);
    Settings::getInstance().saveAvatar(pic, getSelfId().toString());
    emit selfAvatarChanged(pic);
    
    // Broadcast our new avatar!
    // according to tox.h, we need not broadcast this ourselves, but initial testing indicated elsewise
    const uint32_t friendCount = tox_self_get_friend_list_size(tox);
    for (unsigned i=0; i<friendCount; i++)
        ;/// TODO: Send avatar info as a file
}

ToxID Core::getSelfId() const
{
    uint8_t friendAddress[TOX_ADDRESS_SIZE] = {0};
    tox_self_get_address(tox, friendAddress);
    return ToxID::fromString(CFriendAddress::toString(friendAddress));
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

    if (tox_self_set_status_message(tox, cMessage.data(), cMessage.size(), nullptr) == false) {
        emit failedToSetStatusMessage(message);
    } else {
        saveConfiguration();
        emit statusMessageSet(message);
    }
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
            break;
    }

    tox_self_set_status(tox, userstatus);
    saveConfiguration();
    emit statusSet(status);
}

void Core::onFileTransferFinished(ToxFile file)
{
     if (file.direction == file.SENDING)
          emit fileUploadFinished(file.filePath);
     else
          emit fileDownloadFinished(file.filePath);
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

QByteArray Core::loadToxSave(QString path)
{
    QByteArray data;
    loadPath = ""; // if not empty upon return, then user forgot a password and is switching

    QFile configurationFile(path);
    qDebug() << "Core::loadConfiguration: reading from " << path;

    if (!configurationFile.exists()) {
        qWarning() << "The Tox configuration file was not found";
        return data;
    }

    if (!configurationFile.open(QIODevice::ReadOnly)) {
        qCritical() << "File " << path << " cannot be opened";
        return data;
    }

    qint64 fileSize = configurationFile.size();
    if (fileSize > 0) {
        data = configurationFile.readAll();
        /* TODO: Clean this up
        int error = tox_load(tox, reinterpret_cast<uint8_t *>(data.data()), data.size());
        if (error < 0)
        {
            qWarning() << "Core: tox_load failed with error "<< error;
        }
        else if (error == 1) // Encrypted data save
        {
            if (!loadEncryptedSave(data))
            {
                configurationFile.close();

                QString profile = Settings::getInstance().askProfiles();

                if (!profile.isEmpty())
                {
                    loadPath = QDir(Settings::getSettingsDirPath()).filePath(profile + TOX_EXT);
                    Settings::getInstance().switchProfile(profile);
                    HistoryKeeper::resetInstance();
                }
                return false;
            }
        }
        */
    }
    configurationFile.close();

    return data;
}

void Core::saveConfiguration()
{
    if (QThread::currentThread() != coreThread)
        return (void) QMetaObject::invokeMethod(this, "saveConfiguration");

    if (!isReady())
        return;

    QString dir = Settings::getSettingsDirPath();
    QDir directory(dir);
    if (!directory.exists() && !directory.mkpath(directory.absolutePath())) {
        qCritical() << "Error while creating directory " << dir;
        return;
    }

    QString profile = Settings::getInstance().getCurrentProfile();

    if (profile == "")
    { // no profile active; this should only happen on startup, if at all
        profile = sanitize(getUsername());

        if (profile == "") // happens on creation of a new Tox ID
            profile = getIDString();

        Settings::getInstance().switchProfile(profile);
    }

    QString path = directory.filePath(profile + TOX_EXT);

    saveConfiguration(path);
}

void Core::switchConfiguration(const QString& profile)
{
    if (profile.isEmpty())
        qDebug() << "Core: creating new Id";
    else
        qDebug() << "Core: switching from" << Settings::getInstance().getCurrentProfile() << "to" << profile;

    saveConfiguration();
    saveCurrentInformation(); // part of a hack, see core.h

    ready = false;
    GUI::setEnabled(false);
    clearPassword(ptMain);
    clearPassword(ptHistory);

    toxTimer->stop();
    deadifyTox();

    emit selfAvatarChanged(QPixmap(":/img/contact_dark.svg"));
    emit blockingClearContacts(); // we need this to block, but signals are required for thread safety

    if (profile.isEmpty())
        loadPath = "";
    else
        loadPath = QDir(Settings::getSettingsDirPath()).filePath(profile + TOX_EXT);

    Settings::getInstance().switchProfile(profile);
    HistoryKeeper::resetInstance();

    start();
    if (isReady())
        GUI::setEnabled(true);
}

void Core::loadFriends()
{
    const uint32_t friendCount = tox_self_get_friend_list_size(tox);
    if (friendCount > 0) {
        // assuming there are not that many friends to fill up the whole stack
        uint32_t *ids = new uint32_t[friendCount];
        tox_self_get_friend_list(tox, ids);
        uint8_t clientId[TOX_PUBLIC_KEY_SIZE];
        for (int32_t i = 0; i < static_cast<int32_t>(friendCount); ++i) {
            if (tox_friend_get_public_key(tox, ids[i], clientId, nullptr)) {
                emit friendAdded(ids[i], CUserId::toString(clientId));

                const size_t nameSize = tox_friend_get_name_size(tox, ids[i], nullptr);
                if (nameSize != SIZE_MAX) {
                    uint8_t *name = new uint8_t[nameSize];
                    if (tox_friend_get_name(tox, ids[i], name, nullptr))
                        emit friendUsernameChanged(ids[i], CString::toString(name, nameSize));
                    delete[] name;
                }

                const size_t statusMessageSize = tox_friend_get_status_message_size(tox, ids[i], nullptr);
                if (statusMessageSize != SIZE_MAX) {
                    uint8_t *statusMessage = new uint8_t[statusMessageSize];
                    if (tox_friend_get_status_message(tox, ids[i], statusMessage, nullptr)) {
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
    if (lastOnline != UINT64_MAX) {
        emit friendLastSeenChanged(friendId, QDateTime::fromTime_t(lastOnline));
    }
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
        qWarning() << "Core::getGroupPeerName: Unknown error";
        return name;
    }
    name = CString::toString(nameArray, length);
    return name;
}

ToxID Core::getGroupPeerToxID(int groupId, int peerId) const
{
    ToxID peerToxID;

    uint8_t rawID[TOX_PUBLIC_KEY_SIZE];
    int res = tox_group_peer_pubkey(tox, groupId, peerId, rawID);
    if (res == -1)
    {
        qWarning() << "Core::getGroupPeerToxID: Unknown error";
        return peerToxID;
    }

    peerToxID = ToxID::fromString(CUserId::toString(rawID));
    return peerToxID;
}

QList<QString> Core::getGroupPeerNames(int groupId) const
{
    QList<QString> names;
    int nPeers = getGroupNumberPeers(groupId);
    if (nPeers == -1)
    {
        qWarning() << "Core::getGroupPeerNames: Unable to get number of peers";
        return names;
    }
    uint8_t namesArray[nPeers][TOX_MAX_NAME_LENGTH];
    uint16_t* lengths = new uint16_t[nPeers];
    int result = tox_group_get_names(tox, groupId, namesArray, lengths, nPeers);
    if (result != nPeers)
    {
        qWarning() << "Core::getGroupPeerNames: Unexpected result";
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
        qWarning() << "Core::joinGroupchat: Unknown groupchat type "<<type;
        return -1;
    }
}

void Core::quitGroupChat(int groupId) const
{
    tox_del_groupchat(tox, groupId);
}

void Core::removeFileFromQueue(bool sendQueue, uint32_t friendId, uint32_t fileId)
{
    bool found = false;
    if (sendQueue)
    {
        for (int i=0; i<fileSendQueue.size();)
        {
            if (fileSendQueue[i].friendId == friendId && fileSendQueue[i].fileNum == fileId)
            {
                found = true;
                fileSendQueue[i].file->close();
                delete fileSendQueue[i].file;
                fileSendQueue.removeAt(i);
                continue;
            }
            i++;
        }
    }
    else
    {
        for (int i=0; i<fileRecvQueue.size();)
        {
            if (fileRecvQueue[i].friendId == friendId && fileRecvQueue[i].fileNum == fileId)
            {
                found = true;
                fileRecvQueue[i].file->close();
                delete fileRecvQueue[i].file;
                fileRecvQueue.removeAt(i);
                continue;
            }
            i++;
        }
    }
    if (!found)
        qWarning() << "Core::removeFileFromQueue: No such file in queue";
}

void Core::sendAllFileData(Core *core, ToxFile* file)
{
    assert(0);
    /** TODO: Reimplement file transfers using tox_file_chunk_request_cb
    if (file->status == ToxFile::PAUSED)
    {
        file->sendTimer->start(5+TOX_FILE_INTERVAL);
        return;
    }
    else if (file->status == ToxFile::STOPPED)
    {
        qWarning("Core::sendAllFileData: File is stopped");
        file->sendTimer->disconnect();
        delete file->sendTimer;
        file->sendTimer = nullptr;
        return;
    }
    emit core->fileTransferInfo(*file);
//    qApp->processEvents();
    long long chunkSize = tox_file_data_size(core->tox, file->friendId);
    if (chunkSize == -1)
    {
        qWarning("Core::fileHeartbeat: Error getting preffered chunk size, aborting file send");
        file->status = ToxFile::STOPPED;
        emit core->fileTransferCancelled(*file);
        tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr);
        removeFileFromQueue(true, file->friendId, file->fileNum);
        return;
    }
    //qDebug() << "chunkSize: " << chunkSize;
    chunkSize = std::min(chunkSize, file->filesize);
    uint8_t* data = new uint8_t[chunkSize];
    file->file->seek(file->bytesSent);
    int readSize = file->file->read((char*)data, chunkSize);
    if (readSize == -1)
    {
        qWarning() << QString("Core::sendAllFileData: Error reading from file: %1").arg(file->file->errorString());
        delete[] data;
        file->status = ToxFile::STOPPED;
        emit core->fileTransferCancelled(*file);
        tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr);
        removeFileFromQueue(true, file->friendId, file->fileNum);
        return;
    }
    else if (readSize == 0)
    {
        qWarning() << QString("Core::sendAllFileData: Nothing to read from file: %1").arg(file->file->errorString());
        delete[] data;
        file->status = ToxFile::STOPPED;
        emit core->fileTransferCancelled(*file);
        tox_file_control(core->tox, file->friendId, file->fileNum, TOX_FILE_CONTROL_CANCEL, nullptr);
        removeFileFromQueue(true, file->friendId, file->fileNum);
        return;
    }
    if (tox_file_send_data(core->tox, file->friendId, file->fileNum, data, readSize) == -1)
    {
        //qWarning("Core::fileHeartbeat: Error sending data chunk");
        //core->process();
        delete[] data;
        //QThread::msleep(1);
        file->sendTimer->start(1+TOX_FILE_INTERVAL);
        return;
    }
    delete[] data;
    file->bytesSent += readSize;
    //qDebug() << QString("Core::fileHeartbeat: sent %1/%2 bytes").arg(file->bytesSent).arg(file->fileData.size());

    if (file->bytesSent < file->filesize)
    {
        file->sendTimer->start(TOX_FILE_INTERVAL);
        return;
    }
    else
    {
        //qDebug("Core: File transfer finished");
        file->sendTimer->disconnect();
        delete file->sendTimer;
        file->sendTimer = nullptr;
        /// TODO: Review this, we can't send finished messages anymore, but maybe we don't need to
        ///tox_file_control(core->tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_FINISHED, nullptr, 0);
        //emit core->fileTransferFinished(*file);
    }
    */
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
        qWarning() << "Core::createGroup: Unknown type "<<type;
    }
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
    {
        return false;
    }

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
    if (!tox_friend_get_public_key(tox, friendNumber, rawid, nullptr)) {
        qWarning() << "Core::getFriendAddress: Getting public key failed";
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
    if (namesize == SIZE_MAX) {
        qWarning() << "Core::getFriendUsername: Failed to get name size for friend "<<friendnumber;
        return QString();
    }
    uint8_t* name = new uint8_t[namesize];
    tox_friend_get_name(tox, friendnumber, name, nullptr);
    QString sname = CString::toString(name, namesize);
    delete name;
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

QString Core::getPeerName(const ToxID& id) const
{
    QString name;
    CUserId cid(id.toString());

    uint32_t friendId = tox_friend_by_public_key(tox, (uint8_t*)cid.data(), nullptr);
    if (friendId == UINT32_MAX)
    {
        qWarning() << "Core::getPeerName: No such peer "+id.toString();
        return name;
    }

    const size_t nameSize = tox_friend_get_name_size(tox, friendId, nullptr);
    if (nameSize == SIZE_MAX)
    {
        //qDebug() << "Core::getPeerName: Can't get name of friend "+QString().setNum(friendId)+" ("+id.toString()+")";
        return name;
    }

    uint8_t* cname = new uint8_t[nameSize<TOX_MAX_NAME_LENGTH ? TOX_MAX_NAME_LENGTH : nameSize];
    if (tox_friend_get_name(tox, friendId, cname, nullptr) == false)
    {
        qWarning() << "Core::getPeerName: Can't get name of friend "+QString().setNum(friendId)+" ("+id.toString()+")";
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
        call.alSources.clear();

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
