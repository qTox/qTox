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
#include "misc/cdata.h"
#include "misc/cstring.h"
#include "misc/settings.h"
#include "widget/widget.h"

#include <tox/tox.h>

#include <ctime>
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
#include <QMessageBox>

const QString Core::CONFIG_FILE_NAME = "data";
QList<ToxFile> Core::fileSendQueue;
QList<ToxFile> Core::fileRecvQueue;

Core::Core(Camera* cam, QThread *coreThread) :
    tox(nullptr), camera(cam)
{
    videobuf = new uint8_t[videobufsize];
    videoBusyness=0;

    toxTimer = new QTimer(this);
    toxTimer->setSingleShot(true);
    connect(toxTimer, &QTimer::timeout, this, &Core::process);
    //connect(fileTimer, &QTimer::timeout, this, &Core::fileHeartbeat);
    connect(&Settings::getInstance(), &Settings::dhtServerListChanged, this, &Core::process);
    connect(this, SIGNAL(fileTransferFinished(ToxFile)), this, SLOT(onFileTransferFinished(ToxFile)));

    for (int i=0; i<TOXAV_MAX_CALLS;i++)
    {
        calls[i].sendAudioTimer = new QTimer();
        calls[i].sendVideoTimer = new QTimer();
        calls[i].sendAudioTimer->moveToThread(coreThread);
        calls[i].sendVideoTimer->moveToThread(coreThread);
        connect(calls[i].sendVideoTimer, &QTimer::timeout, [this,i](){sendCallVideo(i);});
    }

    // OpenAL init
    alOutDev = alcOpenDevice(nullptr);
    if (!alOutDev)
    {
        qWarning() << "Core: Cannot open output audio device";
    }
    else
    {
        alContext=alcCreateContext(alOutDev,nullptr);
        if (!alcMakeContextCurrent(alContext))
        {
            qWarning() << "Core: Cannot create output audio context";
            alcCloseDevice(alOutDev);
        }
        else
            alGenSources(1, &alMainSource);
    }
    alInDev = alcCaptureOpenDevice(NULL,av_DefaultSettings.audio_sample_rate, AL_FORMAT_MONO16,
                                   (av_DefaultSettings.audio_frame_duration * av_DefaultSettings.audio_sample_rate * 4) / 1000);
    if (!alInDev)
        qWarning() << "Core: Cannot open input audio device";
}

Core::~Core()
{
    if (tox) {
        saveConfiguration();
        toxav_kill(toxav);
        tox_kill(tox);
    }

    if (videobuf)
    {
        delete[] videobuf;
        videobuf=nullptr;
    }

    if (alContext)
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(alContext);
    }
    if (alOutDev)
        alcCloseDevice(alOutDev);
    if (alInDev)
        alcCaptureCloseDevice(alInDev);
}

Core* Core::getInstance()
{
    return Widget::getInstance()->getCore();
}

void Core::start()
{
    // IPv6 needed for LAN discovery, but can crash some weird routers. On by default, can be disabled in options.
    bool enableIPv6 = Settings::getInstance().getEnableIPv6();
    bool forceTCP = Settings::getInstance().getForceTCP();
    QString proxyAddr = Settings::getInstance().getProxyAddr();
    int proxyPort = Settings::getInstance().getProxyPort();
    if (enableIPv6)
        qDebug() << "Core starting with IPv6 enabled";
    else
        qWarning() << "Core starting with IPv6 disabled. LAN discovery may not work properly.";

    Tox_Options toxOptions;
    toxOptions.ipv6enabled = enableIPv6;
    toxOptions.udp_disabled = forceTCP;
    if (proxyAddr.length() > 255)
    {
        qWarning() << "Core: proxy address" << proxyAddr << "is too long";
        toxOptions.proxy_enabled = false;
        toxOptions.proxy_address[0] = 0;
        toxOptions.proxy_port = 0;
    }
    else if (proxyAddr != "" && proxyPort > 0)
    {
        qDebug() << "Core: using proxy" << proxyAddr << ":" << proxyPort;
        toxOptions.proxy_enabled = true;
        uint16_t sz = CString::fromString(proxyAddr, (unsigned char*)toxOptions.proxy_address);
        toxOptions.proxy_address[sz] = 0;
        toxOptions.proxy_port = proxyPort;
    }
    else
    {
        toxOptions.proxy_enabled = false;
        toxOptions.proxy_address[0] = 0;
        toxOptions.proxy_port = 0;
    }

    tox = tox_new(&toxOptions);
    if (tox == nullptr)
    {
        if (enableIPv6) // Fallback to IPv4
        {
            toxOptions.ipv6enabled = false;
            tox = tox_new(&toxOptions);
            if (tox == nullptr)
            {
                if (toxOptions.proxy_enabled)
                {
                    //QMessageBox::critical(Widget::getInstance(), tr("Proxy failure", "popup title"), 
                    //tr("toxcore failed to start with your proxy settings. qTox cannot run; please modify your "
                       //"settings and restart.", "popup text"));
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
        else if (toxOptions.proxy_enabled)
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

    qsrand(time(nullptr));

    if (!loadConfiguration())
    {
        emit failedToStart();
        tox_kill(tox);
        tox = nullptr;
        return;
    }

    tox_callback_friend_request(tox, onFriendRequest, this);
    tox_callback_friend_message(tox, onFriendMessage, this);
    tox_callback_friend_action(tox, onAction, this);
    tox_callback_name_change(tox, onFriendNameChange, this);
    tox_callback_typing_change(tox, onFriendTypingChange, this);
    tox_callback_status_message(tox, onStatusMessageChanged, this);
    tox_callback_user_status(tox, onUserStatusChanged, this);
    tox_callback_connection_status(tox, onConnectionStatusChanged, this);
    tox_callback_group_invite(tox, onGroupInvite, this);
    tox_callback_group_message(tox, onGroupMessage, this);
    tox_callback_group_namelist_change(tox, onGroupNamelistChange, this);
    tox_callback_file_send_request(tox, onFileSendRequestCallback, this);
    tox_callback_file_control(tox, onFileControlCallback, this);
    tox_callback_file_data(tox, onFileDataCallback, this);
    tox_callback_avatar_info(tox, onAvatarInfoCallback, this);
    tox_callback_avatar_data(tox, onAvatarDataCallback, this);

    toxav_register_callstate_callback(toxav, onAvInvite, av_OnInvite, this);
    toxav_register_callstate_callback(toxav, onAvStart, av_OnStart, this);
    toxav_register_callstate_callback(toxav, onAvCancel, av_OnCancel, this);
    toxav_register_callstate_callback(toxav, onAvReject, av_OnReject, this);
    toxav_register_callstate_callback(toxav, onAvEnd, av_OnEnd, this);
    toxav_register_callstate_callback(toxav, onAvRinging, av_OnRinging, this);
    toxav_register_callstate_callback(toxav, onAvStarting, av_OnStarting, this);
    toxav_register_callstate_callback(toxav, onAvEnding, av_OnEnding, this);
    toxav_register_callstate_callback(toxav, onAvMediaChange, av_OnMediaChange, this);
    toxav_register_callstate_callback(toxav, onAvRequestTimeout, av_OnRequestTimeout, this);
    toxav_register_callstate_callback(toxav, onAvPeerTimeout, av_OnPeerTimeout, this);

    toxav_register_audio_recv_callback(toxav, playCallAudio, this);
    toxav_register_video_recv_callback(toxav, playCallVideo, this);

    uint8_t friendAddress[TOX_FRIEND_ADDRESS_SIZE];
    tox_get_address(tox, friendAddress);
    emit friendAddressGenerated(CFriendAddress::toString(friendAddress));

    QPixmap pic = Settings::getInstance().getSavedAvatar(getSelfId().toString());
    if (!pic.isNull() && !pic.size().isEmpty())
    {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        pic.save(&buffer, "PNG");
        buffer.close();
        setAvatar(TOX_AVATAR_FORMAT_PNG, data);
    }
    else
        qDebug() << "Core: Error loading self avatar";
    
    process(); // starts its own timer
}

/* Using the now commented out statements in checkConnection(), I watched how
 * many ticks disconnects-after-initial-connect lasted. Out of roughly 15 trials,
 * 5 disconnected; 4 were DCd for less than 20 ticks, while the 5th was ~50 ticks.
 * So I set the tolerance here at 25, and initial DCs should be very rare now.
 * This should be able to go to 50 or 100 without affecting legitimate disconnects'
 * downtime, but lets be conservative for now. Edit: now 40.
 */
#define CORE_DISCONNECT_TOLERANCE 40

void Core::process()
{
    if (!tox)
        return;

    static int tolerance = CORE_DISCONNECT_TOLERANCE;
    tox_do(tox);

#ifdef DEBUG
    //we want to see the debug messages immediately
    fflush(stdout);
#endif

    if (checkConnection())
        tolerance = CORE_DISCONNECT_TOLERANCE;
    else if (!(--tolerance))
    {
        bootstrapDht();
    }

    toxTimer->start(tox_do_interval(tox));
}

bool Core::checkConnection()
{
    static bool isConnected = false;
    //static int count = 0;
    bool toxConnected = tox_isconnected(tox);

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
    static int j = qrand() % listSize;

    qDebug() << "Core: Bootstraping to the DHT ...";

    int i=0;
    while (i < 2) // i think the more we bootstrap, the more we jitter because the more we overwrite nodes
    {
        const Settings::DhtServer& dhtServer = dhtServerList[j % listSize];
        if (tox_bootstrap_from_address(tox, dhtServer.address.toLatin1().data(),
            dhtServer.port, CUserId(dhtServer.userId).data()) == 1)
            qDebug() << QString("Core: Bootstraping from ")+dhtServer.name+QString(", addr ")+dhtServer.address.toLatin1().data()
                        +QString(", port ")+QString().setNum(dhtServer.port);
        else
            qDebug() << "Core: Error bootstraping from "+dhtServer.name;

        j++;
        i++;
    }
}

void Core::onFriendRequest(Tox*/* tox*/, const uint8_t* cUserId, const uint8_t* cMessage, uint16_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendRequestReceived(CUserId::toString(cUserId), CString::toString(cMessage, cMessageSize));
}

void Core::onFriendMessage(Tox*/* tox*/, int friendId, const uint8_t* cMessage, uint16_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendMessageReceived(friendId, CString::toString(cMessage, cMessageSize), false);
}

void Core::onFriendNameChange(Tox*/* tox*/, int friendId, const uint8_t* cName, uint16_t cNameSize, void* core)
{
    emit static_cast<Core*>(core)->friendUsernameChanged(friendId, CString::toString(cName, cNameSize));
}

void Core::onFriendTypingChange(Tox*/* tox*/, int friendId, uint8_t isTyping, void *core)
{
    emit static_cast<Core*>(core)->friendTypingChanged(friendId, isTyping ? true : false);
}

void Core::onStatusMessageChanged(Tox*/* tox*/, int friendId, const uint8_t* cMessage, uint16_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendStatusMessageChanged(friendId, CString::toString(cMessage, cMessageSize));
}

void Core::onUserStatusChanged(Tox*/* tox*/, int friendId, uint8_t userstatus, void* core)
{
    Status status;
    switch (userstatus) {
        case TOX_USERSTATUS_NONE:
            status = Status::Online;
            break;
        case TOX_USERSTATUS_AWAY:
            status = Status::Away;
            break;
        case TOX_USERSTATUS_BUSY:
            status = Status::Busy;
            break;
        default:
            status = Status::Online;
            break;
    }

    if (status == Status::Online || status == Status::Away)
        tox_request_avatar_info(static_cast<Core*>(core)->tox, friendId);

    emit static_cast<Core*>(core)->friendStatusChanged(friendId, status);
}

void Core::onConnectionStatusChanged(Tox*/* tox*/, int friendId, uint8_t status, void* core)
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
                tox_file_send_control(static_cast<Core*>(core)->tox, friendId, 1, f.fileNum, TOX_FILECONTROL_RESUME_BROKEN, reinterpret_cast<const uint8_t*>(&f.bytesSent), sizeof(uint64_t));
                emit static_cast<Core*>(core)->fileTransferBrokenUnbroken(f, false);
            }
        }
    }
}

void Core::onAction(Tox*/* tox*/, int friendId, const uint8_t *cMessage, uint16_t cMessageSize, void *core)
{
    emit static_cast<Core*>(core)->friendMessageReceived(friendId, CString::toString(cMessage, cMessageSize), true);
}

void Core::onGroupInvite(Tox*, int friendnumber, const uint8_t *group_public_key, uint16_t length,void *core)
{
    qDebug() << QString("Core: Group invite by %1").arg(friendnumber);
    emit static_cast<Core*>(core)->groupInviteReceived(friendnumber, group_public_key,length);
}

void Core::onGroupMessage(Tox*, int groupnumber, int friendgroupnumber, const uint8_t * message, uint16_t length, void *core)
{
    emit static_cast<Core*>(core)->groupMessageReceived(groupnumber, friendgroupnumber, CString::toString(message, length));
}

void Core::onGroupNamelistChange(Tox*, int groupnumber, int peernumber, uint8_t change, void *core)
{
    qDebug() << QString("Core: Group namelist change %1:%2 %3").arg(groupnumber).arg(peernumber).arg(change);
    emit static_cast<Core*>(core)->groupNamelistChanged(groupnumber, peernumber, change);
}

void Core::onFileSendRequestCallback(Tox*, int32_t friendnumber, uint8_t filenumber, uint64_t filesize,
                                          const uint8_t *filename, uint16_t filename_length, void *core)
{
    qDebug() << QString("Core: Received file request %1 with friend %2").arg(filenumber).arg(friendnumber);

    ToxFile file{filenumber, friendnumber,
                CString::toString(filename,filename_length).toUtf8(), "", ToxFile::RECEIVING};
    file.filesize = filesize;
    fileRecvQueue.append(file);
    emit static_cast<Core*>(core)->fileReceiveRequested(fileRecvQueue.last());
}
void Core::onFileControlCallback(Tox* tox, int32_t friendnumber, uint8_t receive_send, uint8_t filenumber,
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
    if      (receive_send == 1 && control_type == TOX_FILECONTROL_ACCEPT)
    {
        file->status = ToxFile::TRANSMITTING;
        emit static_cast<Core*>(core)->fileTransferAccepted(*file);
        qDebug() << "Core: File control callback, file accepted";
        file->sendTimer = new QTimer(static_cast<Core*>(core));
        connect(file->sendTimer, &QTimer::timeout, std::bind(sendAllFileData,static_cast<Core*>(core), file));
        file->sendTimer->setSingleShot(true);
        file->sendTimer->start(TOX_FILE_INTERVAL);
    }
    else if (receive_send == 1 && control_type == TOX_FILECONTROL_KILL)
    {
        qDebug() << QString("Core::onFileControlCallback: Transfer of file %1 cancelled by friend %2")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferCancelled(file->friendId, file->fileNum, ToxFile::SENDING);
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
    else if (receive_send == 1 && control_type == TOX_FILECONTROL_FINISHED)
    {
        qDebug() << QString("Core::onFileControlCallback: Transfer of file %1 to friend %2 is complete")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferFinished(*file);
        removeFileFromQueue((bool)receive_send, file->friendId, file->fileNum);
    }
    else if (receive_send == 0 && control_type == TOX_FILECONTROL_KILL)
    {
        qDebug() << QString("Core::onFileControlCallback: Transfer of file %1 cancelled by friend %2")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferCancelled(file->friendId, file->fileNum, ToxFile::RECEIVING);
        removeFileFromQueue((bool)receive_send, file->friendId, file->fileNum);
    }
    else if (receive_send == 0 && control_type == TOX_FILECONTROL_FINISHED)
    {
        qDebug() << QString("Core::onFileControlCallback: Reception of file %1 from %2 finished")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferFinished(*file);
        // confirm receive is complete
        tox_file_send_control(tox, file->friendId, 1, file->fileNum, TOX_FILECONTROL_FINISHED, nullptr, 0);
        removeFileFromQueue((bool)receive_send, file->friendId, file->fileNum);
    }
    else if (receive_send == 0 && control_type == TOX_FILECONTROL_ACCEPT)
    {
        if (file->status == ToxFile::BROKEN)
        {
            emit static_cast<Core*>(core)->fileTransferBrokenUnbroken(*file, false);
            file->status = ToxFile::TRANSMITTING;
        }
        emit static_cast<Core*>(core)->fileTransferRemotePausedUnpaused(*file, false);
    }
    else if ((receive_send == 0 || receive_send == 1) && control_type == TOX_FILECONTROL_PAUSE)
    {
        emit static_cast<Core*>(core)->fileTransferRemotePausedUnpaused(*file, true);
    }
    else if (receive_send == 1 && control_type == TOX_FILECONTROL_RESUME_BROKEN)
    {
        if (length != sizeof(uint64_t))
            return;

        qDebug() << "Core::onFileControlCallback: TOX_FILECONTROL_RESUME_BROKEN";

        uint64_t resumePos = *reinterpret_cast<const uint64_t*>(data);

        if (resumePos >= file->filesize)
        {
            qWarning() << "Core::onFileControlCallback: invalid resume position";
            tox_file_send_control(tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_KILL, nullptr, 0); // don't sure about it
            return;
        }

        file->status = ToxFile::TRANSMITTING;
        emit static_cast<Core*>(core)->fileTransferBrokenUnbroken(*file, false);

        file->bytesSent = resumePos;
        tox_file_send_control(tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_ACCEPT, nullptr, 0);
    }
    else
    {
        qDebug() << QString("Core: File control callback, receive_send=%1, control_type=%2")
                    .arg(receive_send).arg(control_type);
    }
}

void Core::onFileDataCallback(Tox*, int32_t friendnumber, uint8_t filenumber, const uint8_t *data, uint16_t length, void *core)
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
    emit static_cast<Core*>(core)->fileTransferInfo(file->friendId, file->fileNum,
                                            file->filesize, file->bytesSent, ToxFile::RECEIVING);
}

void Core::onAvatarInfoCallback(Tox*, int32_t friendnumber, uint8_t format,
                                uint8_t* hash, void* _core)
{
    Core* core = static_cast<Core*>(_core);

    if (format == TOX_AVATAR_FORMAT_NONE)
    {
        qDebug() << "Core: Got null avatar info from" << core->getFriendUsername(friendnumber);
        emit core->friendAvatarRemoved(friendnumber);
        QFile::remove(QDir(Settings::getInstance().getSettingsDirPath()).filePath("avatars/"+core->getFriendAddress(friendnumber).left(64)+".png"));
        QFile::remove(QDir(Settings::getInstance().getSettingsDirPath()).filePath("avatars/"+core->getFriendAddress(friendnumber).left(64)+".hash"));
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
        else
            qDebug() << "Core: Got same avatar info from" << core->getFriendUsername(friendnumber);
    }
}

void Core::onAvatarDataCallback(Tox*, int32_t friendnumber, uint8_t,
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

void Core::acceptFriendRequest(const QString& userId)
{
    int friendId = tox_add_friend_norequest(tox, CUserId(userId).data());
    if (friendId == -1) {
        emit failedToAddFriend(userId);
    } else {
        saveConfiguration();
        emit friendAdded(friendId, userId);
    }
}

void Core::requestFriendship(const QString& friendAddress, const QString& message)
{
    qDebug() << "Core: requesting friendship of "+friendAddress;
    CString cMessage(message);

    int friendId = tox_add_friend(tox, CFriendAddress(friendAddress).data(), cMessage.data(), cMessage.size());
    const QString userId = friendAddress.mid(0, TOX_CLIENT_ID_SIZE * 2);
    if (friendId < 0) {
        emit failedToAddFriend(userId);
    } else {
        // Update our friendAddresses
        bool found=false;
        QList<QString>& friendAddresses = Settings::getInstance().friendAddresses;
        for (QString& addr : friendAddresses)
        {
            if (addr.toUpper().contains(friendAddress))
            {
                addr = friendAddress;
                found = true;
            }
        }
        if (!found)
            friendAddresses.append(friendAddress);
        emit friendAdded(friendId, userId);
    }
    saveConfiguration();
}

void Core::sendMessage(int friendId, const QString& message)
{
    QList<CString> cMessages = splitMessage(message);

    for (auto &cMsg :cMessages)
    {
        int messageId = tox_send_message(tox, friendId, cMsg.data(), cMsg.size());
        if (messageId == 0)
            emit messageSentResult(friendId, message, messageId);
    }
}

void Core::sendAction(int friendId, const QString &action)
{
    CString cMessage(action);
    int ret = tox_send_action(tox, friendId, cMessage.data(), cMessage.size());
    emit actionSentResult(friendId, action, ret);
}

void Core::sendTyping(int friendId, bool typing)
{
    int ret = tox_set_user_is_typing(tox, friendId, typing);
    if (ret == -1)
        emit failedToSetTyping(typing);
}

void Core::sendGroupMessage(int groupId, const QString& message)
{
    QList<CString> cMessages = splitMessage(message);

    for (auto &cMsg :cMessages)
    {
        int ret = tox_group_message_send(tox, groupId, cMsg.data(), cMsg.size());
        if (ret == -1)
            emit groupSentResult(groupId, message, ret);
    }
}

void Core::sendFile(int32_t friendId, QString Filename, QString FilePath, long long filesize)
{
    QByteArray fileName = Filename.toUtf8();
    int fileNum = tox_new_file_sender(tox, friendId, filesize, (uint8_t*)fileName.data(), fileName.size());
    if (fileNum == -1)
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

void Core::pauseResumeFileSend(int friendId, int fileNum)
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
        emit fileTransferPaused(file->friendId, file->fileNum, ToxFile::SENDING);
        tox_file_send_control(tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_PAUSE, nullptr, 0);
    }
    else if (file->status == ToxFile::PAUSED)
    {
        file->status = ToxFile::TRANSMITTING;
        emit fileTransferAccepted(*file);
        tox_file_send_control(tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_ACCEPT, nullptr, 0);
    }
    else
        qWarning() << "Core::pauseResumeFileSend: File is stopped";
}

void Core::pauseResumeFileRecv(int friendId, int fileNum)
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
        emit fileTransferPaused(file->friendId, file->fileNum, ToxFile::RECEIVING);
        tox_file_send_control(tox, file->friendId, 1, file->fileNum, TOX_FILECONTROL_PAUSE, nullptr, 0);
    }
    else if (file->status == ToxFile::PAUSED)
    {
        file->status = ToxFile::TRANSMITTING;
        emit fileTransferAccepted(*file);
        tox_file_send_control(tox, file->friendId, 1, file->fileNum, TOX_FILECONTROL_ACCEPT, nullptr, 0);
    }
    else
        qWarning() << "Core::pauseResumeFileRecv: File is stopped or broken";
}

void Core::cancelFileSend(int friendId, int fileNum)
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
    emit fileTransferCancelled(file->friendId, file->fileNum, ToxFile::SENDING);
    tox_file_send_control(tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_KILL, nullptr, 0);
    while (file->sendTimer) QThread::msleep(1); // Wait until sendAllFileData returns before deleting
    removeFileFromQueue(true, friendId, fileNum);
}

void Core::cancelFileRecv(int friendId, int fileNum)
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
    emit fileTransferCancelled(file->friendId, file->fileNum, ToxFile::RECEIVING);
    tox_file_send_control(tox, file->friendId, 1, file->fileNum, TOX_FILECONTROL_KILL, nullptr, 0);
    removeFileFromQueue(true, friendId, fileNum);
}

void Core::rejectFileRecvRequest(int friendId, int fileNum)
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
    emit fileTransferCancelled(file->friendId, file->fileNum, ToxFile::SENDING);
    tox_file_send_control(tox, file->friendId, 1, file->fileNum, TOX_FILECONTROL_KILL, nullptr, 0);
    removeFileFromQueue(false, friendId, fileNum);
}

void Core::acceptFileRecvRequest(int friendId, int fileNum, QString path)
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
    tox_file_send_control(tox, file->friendId, 1, file->fileNum, TOX_FILECONTROL_ACCEPT, nullptr, 0);
}

void Core::removeFriend(int friendId)
{
    if (tox_del_friend(tox, friendId) == -1) {
        emit failedToRemoveFriend(friendId);
    } else {
        saveConfiguration();
        emit friendRemoved(friendId);
    }
}

void Core::removeGroup(int groupId)
{
    tox_del_groupchat(tox, groupId);
}

QString Core::getUsername()
{
    int size = tox_get_self_name_size(tox);
    uint8_t* name = new uint8_t[size];
    if (tox_get_self_name(tox, name) == size)
        return QString(CString::toString(name, size));
    else
        return QString();
    delete[] name;
}

void Core::setUsername(const QString& username)
{
    CString cUsername(username);

    if (tox_set_name(tox, cUsername.data(), cUsername.size()) == -1) {
        emit failedToSetUsername(username);
    } else {
        saveConfiguration();
        emit usernameSet(username);
    }
}

void Core::setAvatar(uint8_t format, const QByteArray& data)
{
    if (tox_set_avatar(tox, format, (uint8_t*)data.constData(), data.size()) != 0)
    {
        qWarning() << "Core: Failed to set self avatar";
        return;
    }

    QPixmap pic;
    pic.loadFromData(data);
    Settings::getInstance().saveAvatar(pic, getSelfId().toString());
    emit selfAvatarChanged(pic);
    
    // Broadcast our new avatar!
    // according to tox.h, we need not broadcast this ourselves, but initial testing indicated elsewise
    const uint32_t friendCount = tox_count_friendlist(tox);;
    for (unsigned i=0; i<friendCount; i++)
        tox_send_avatar_info(tox, i);
}

ToxID Core::getSelfId()
{
    uint8_t friendAddress[TOX_FRIEND_ADDRESS_SIZE];
    tox_get_address(tox, friendAddress);
    return ToxID::fromString(CFriendAddress::toString(friendAddress));
}

QString Core::getStatusMessage()
{
    int size = tox_get_self_status_message_size(tox);
    uint8_t* name = new uint8_t[size];
    if (tox_get_self_status_message(tox, name, size) == size)
        return QString(CString::toString(name, size));
    else
        return QString();
    delete[] name;
}

void Core::setStatusMessage(const QString& message)
{
    CString cMessage(message);

    if (tox_set_status_message(tox, cMessage.data(), cMessage.size()) == -1) {
        emit failedToSetStatusMessage(message);
    } else {
        saveConfiguration();
        emit statusMessageSet(message);
    }
}

void Core::setStatus(Status status)
{
    TOX_USERSTATUS userstatus;
    switch (status) {
        case Status::Online:
            userstatus = TOX_USERSTATUS_NONE;
            break;
        case Status::Away:
            userstatus = TOX_USERSTATUS_AWAY;
            break;
        case Status::Busy:
            userstatus = TOX_USERSTATUS_BUSY;
            break;
        default:
            userstatus = TOX_USERSTATUS_INVALID;
            break;
    }

    if (tox_set_user_status(tox, userstatus) == 0) {
        saveConfiguration();
        emit statusSet(status);
    } else {
        emit failedToSetStatus(status);
    }
}

void Core::onFileTransferFinished(ToxFile file)
{
     if (file.direction == file.SENDING)
          emit fileUploadFinished(file.filePath);
     else
          emit fileDownloadFinished(file.filePath);
}

bool Core::loadConfiguration()
{
    QString path = QDir(Settings::getSettingsDirPath()).filePath(CONFIG_FILE_NAME);

    QFile configurationFile(path);

    if (!configurationFile.exists()) {
        qWarning() << "The Tox configuration file was not found";
        return true;
    }

    if (!configurationFile.open(QIODevice::ReadOnly)) {
        qCritical() << "File " << path << " cannot be opened";
        return true;
    }

    qint64 fileSize = configurationFile.size();
    if (fileSize > 0) {
        QByteArray data = configurationFile.readAll();
        int error = tox_load(tox, reinterpret_cast<uint8_t *>(data.data()), data.size());
        if (error < 0)
        {
            qWarning() << "Core: tox_load failed with error "<<error;
        }
        else if (error == 1) // Encrypted data save
        {
            qWarning() << "Core: Can not open encrypted tox save";
            if (QMessageBox::Ok != QMessageBox::warning(nullptr, tr("Encrypted profile"),
                tr("Your tox profile seems to be encrypted, qTox can't open it\nDo you want to erase this profile ?"),
                QMessageBox::Ok | QMessageBox::Cancel))
            {
                qWarning() << "Core: Couldn't open encrypted save, giving up";
                configurationFile.close();
                return false;
            }
        }
    }

    configurationFile.close();

    // set GUI with user and statusmsg
    QString name = getUsername();
    if (name != "")
        emit usernameSet(name);
    
    QString msg = getStatusMessage();
    if (msg != "")
        emit statusMessageSet(msg);

    loadFriends();
    return true;
}

void Core::saveConfiguration()
{
    Settings::getInstance().save();
    
    if (!tox)
    {
        qWarning() << "Core::saveConfiguration: Tox not started, aborting!";
        return;
    }

    QString path = Settings::getSettingsDirPath();

    QDir directory(path);

    if (!directory.exists() && !directory.mkpath(directory.absolutePath())) {
        qCritical() << "Error while creating directory " << path;
        return;
    }

    path = directory.filePath(CONFIG_FILE_NAME);
    QSaveFile configurationFile(path);
    if (!configurationFile.open(QIODevice::WriteOnly)) {
        qCritical() << "File " << path << " cannot be opened";
        return;
    }

        qDebug() << "Core: Saving";

    uint32_t fileSize = tox_size(tox);
    if (fileSize > 0 && fileSize <= INT32_MAX) {
        uint8_t *data = new uint8_t[fileSize];
        tox_save(tox, data);
        configurationFile.write(reinterpret_cast<char *>(data), fileSize);
        configurationFile.commit();
        delete[] data;
    }
}

void Core::loadFriends()
{
    const uint32_t friendCount = tox_count_friendlist(tox);
    if (friendCount > 0) {
        // assuming there are not that many friends to fill up the whole stack
        int32_t *ids = new int32_t[friendCount];
        tox_get_friendlist(tox, ids, friendCount);
        uint8_t clientId[TOX_CLIENT_ID_SIZE];
        for (int32_t i = 0; i < static_cast<int32_t>(friendCount); ++i) {
            if (tox_get_client_id(tox, ids[i], clientId) == 0) {
                emit friendAdded(ids[i], CUserId::toString(clientId));

                const int nameSize = tox_get_name_size(tox, ids[i]);
                if (nameSize > 0) {
                    uint8_t *name = new uint8_t[nameSize];
                    if (tox_get_name(tox, ids[i], name) == nameSize) {
                        emit friendUsernameChanged(ids[i], CString::toString(name, nameSize));
                    }
                    delete[] name;
                }

                const int statusMessageSize = tox_get_status_message_size(tox, ids[i]);
                if (statusMessageSize > 0) {
                    uint8_t *statusMessage = new uint8_t[statusMessageSize];
                    if (tox_get_status_message(tox, ids[i], statusMessage, statusMessageSize) == statusMessageSize) {
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

void Core::checkLastOnline(int friendId) {
    const uint64_t lastOnline = tox_get_last_online(tox, friendId);
    if (lastOnline > 0) {
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

int Core::joinGroupchat(int32_t friendnumber, const uint8_t* friend_group_public_key,uint16_t length) const
{
    qDebug() << QString("Trying to join groupchat invite by friend %1").arg(friendnumber);
    return tox_join_groupchat(tox, friendnumber, friend_group_public_key,length);
}

void Core::quitGroupChat(int groupId) const
{
    tox_del_groupchat(tox, groupId);
}

void Core::removeFileFromQueue(bool sendQueue, int friendId, int fileId)
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
    emit core->fileTransferInfo(file->friendId, file->fileNum, file->filesize, file->bytesSent, ToxFile::SENDING);
    qApp->processEvents();
    long long chunkSize = tox_file_data_size(core->tox, file->friendId);
    if (chunkSize == -1)
    {
        qWarning("Core::fileHeartbeat: Error getting preffered chunk size, aborting file send");
        file->status = ToxFile::STOPPED;
        emit core->fileTransferCancelled(file->friendId, file->fileNum, ToxFile::SENDING);
        tox_file_send_control(core->tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_KILL, nullptr, 0);
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
        emit core->fileTransferCancelled(file->friendId, file->fileNum, ToxFile::SENDING);
        tox_file_send_control(core->tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_KILL, nullptr, 0);
        removeFileFromQueue(true, file->friendId, file->fileNum);
        return;
    }
    else if (readSize == 0)
    {
        qWarning() << QString("Core::sendAllFileData: Nothing to read from file: %1").arg(file->file->errorString());
        delete[] data;
        file->status = ToxFile::STOPPED;
        emit core->fileTransferCancelled(file->friendId, file->fileNum, ToxFile::SENDING);
        tox_file_send_control(core->tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_KILL, nullptr, 0);
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
        tox_file_send_control(core->tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_FINISHED, nullptr, 0);
        //emit core->fileTransferFinished(*file);
    }
}

void Core::groupInviteFriend(int friendId, int groupId)
{
    tox_invite_friend(tox, friendId, groupId);
}

void Core::createGroup()
{
    emit emptyGroupCreated(tox_add_groupchat(tox));
}

QString Core::getFriendAddress(int friendNumber) const
{
    // If we don't know the full address of the client, return just the id, otherwise get the full address
    uint8_t rawid[TOX_CLIENT_ID_SIZE];
    tox_get_client_id(tox, friendNumber, rawid);
    QByteArray data((char*)rawid,TOX_CLIENT_ID_SIZE);
    QString id = data.toHex().toUpper();

    QList<QString>& friendAddresses = Settings::getInstance().friendAddresses;
    for (QString addr : friendAddresses)
        if (addr.toUpper().contains(id))
            return addr;

    return id;
}

QString Core::getFriendUsername(int friendnumber) const
{
    uint8_t name[TOX_MAX_NAME_LENGTH];
    tox_get_name(tox, friendnumber, name);
    return CString::toString(name, tox_get_name_size(tox, friendnumber));
}

QList<CString> Core::splitMessage(const QString &message)
{
    QList<CString> splittedMsgs;
    QByteArray ba_message(message.toUtf8());

    while (ba_message.size() > TOX_MAX_MESSAGE_LENGTH)
    {
        int splitPos = ba_message.lastIndexOf(' ', TOX_MAX_MESSAGE_LENGTH - 1);
        if (splitPos <= 0)
        {
            splitPos = TOX_MAX_MESSAGE_LENGTH;
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
