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
#include "cdata.h"
#include "cstring.h"
#include "settings.h"
#include "widget/widget.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>
#include <QtEndian>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

const QString Core::CONFIG_FILE_NAME = "data";
QList<ToxFile> Core::fileSendQueue;
QList<ToxFile> Core::fileRecvQueue;
ToxCall Core::calls[TOXAV_MAX_CALLS];

Core::Core(Camera* cam, QThread *coreThread) :
    tox(nullptr), camera(cam)
{
    toxTimer = new QTimer(this);
    toxTimer->setSingleShot(true);
    //saveTimer = new QTimer(this);
    //saveTimer->start(TOX_SAVE_INTERVAL);
    //fileTimer = new QTimer(this);
    //fileTimer->start(TOX_FILE_INTERVAL);
    bootstrapTimer = new QTimer(this);
    bootstrapTimer->start(TOX_BOOTSTRAP_INTERVAL);
    connect(toxTimer, &QTimer::timeout, this, &Core::process);
    //connect(saveTimer, &QTimer::timeout, this, &Core::saveConfiguration); //Disable save timer in favor of saving on events
    //connect(fileTimer, &QTimer::timeout, this, &Core::fileHeartbeat);
    connect(bootstrapTimer, &QTimer::timeout, this, &Core::onBootstrapTimer);
    connect(&Settings::getInstance(), &Settings::dhtServerListChanged, this, &Core::bootstrapDht);

    for (int i=0; i<TOXAV_MAX_CALLS;i++)
    {
        calls[i].playAudioTimer = new QTimer(this);
        calls[i].sendAudioTimer = new QTimer(this);
        calls[i].playVideoTimer = new QTimer(this);
        calls[i].sendVideoTimer = new QTimer(this);
        calls[i].audioBuffer.moveToThread(coreThread);
        calls[i].playAudioTimer->moveToThread(coreThread);
        calls[i].sendAudioTimer->moveToThread(coreThread);
        calls[i].playVideoTimer->moveToThread(coreThread);
        calls[i].sendVideoTimer->moveToThread(coreThread);
        connect(calls[i].playVideoTimer, &QTimer::timeout, [this,i](){playCallVideo(i);});
        connect(calls[i].sendVideoTimer, &QTimer::timeout, [this,i](){sendCallVideo(i);});
    }
}

Core::~Core()
{
    if (tox) {
        saveConfiguration();
        toxav_kill(toxav);
        tox_kill(tox);
    }
}

void Core::start()
{
    // IPv6 needed for LAN discovery, but can crash some weird routers. On by default, can be disabled in options.
    bool enableIPv6 = Settings::getInstance().getEnableIPv6();
    if (enableIPv6)
        qDebug() << "Core starting with IPv6 enabled";
    else
        qWarning() << "Core starting with IPv6 disabled. LAN discovery may not work properly.";
    tox = tox_new(enableIPv6);
    if (tox == nullptr)
    {
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

    loadConfiguration();

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

    toxav_register_callstate_callback(onAvInvite, av_OnInvite, this);
    toxav_register_callstate_callback(onAvStart, av_OnStart, this);
    toxav_register_callstate_callback(onAvCancel, av_OnCancel, this);
    toxav_register_callstate_callback(onAvReject, av_OnReject, this);
    toxav_register_callstate_callback(onAvEnd, av_OnEnd, this);
    toxav_register_callstate_callback(onAvRinging, av_OnRinging, this);
    toxav_register_callstate_callback(onAvStarting, av_OnStarting, this);
    toxav_register_callstate_callback(onAvEnding, av_OnEnding, this);
    toxav_register_callstate_callback(onAvError, av_OnError, this);
    toxav_register_callstate_callback(onAvRequestTimeout, av_OnRequestTimeout, this);
    toxav_register_callstate_callback(onAvPeerTimeout, av_OnPeerTimeout, this);

    uint8_t friendAddress[TOX_FRIEND_ADDRESS_SIZE];
    tox_get_address(tox, friendAddress);

    emit friendAddressGenerated(CFriendAddress::toString(friendAddress));

    bootstrapDht();

    toxTimer->start(tox_do_interval(tox));
}

void Core::onBootstrapTimer()
{
    if (!tox)
        return;
    if(!tox_isconnected(tox))
        bootstrapDht();
}

void Core::onFriendRequest(Tox*/* tox*/, const uint8_t* cUserId, const uint8_t* cMessage, uint16_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendRequestReceived(CUserId::toString(cUserId), CString::toString(cMessage, cMessageSize));
}

void Core::onFriendMessage(Tox*/* tox*/, int friendId, const uint8_t* cMessage, uint16_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendMessageReceived(friendId, CString::toString(cMessage, cMessageSize));
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
    emit static_cast<Core*>(core)->friendStatusChanged(friendId, status);
}

void Core::onConnectionStatusChanged(Tox*/* tox*/, int friendId, uint8_t status, void* core)
{
    Status friendStatus = status ? Status::Online : Status::Offline;
    emit static_cast<Core*>(core)->friendStatusChanged(friendId, friendStatus);
    if (friendStatus == Status::Offline) {
        static_cast<Core*>(core)->checkLastOnline(friendId);
    }
}

void Core::onAction(Tox*/* tox*/, int friendId, const uint8_t *cMessage, uint16_t cMessageSize, void *core)
{
    emit static_cast<Core*>(core)->actionReceived(friendId, CString::toString(cMessage, cMessageSize));
}

void Core::onGroupInvite(Tox*, int friendnumber, const uint8_t *group_public_key, void *core)
{
    qDebug() << QString("Core: Group invite by %1").arg(friendnumber);
    emit static_cast<Core*>(core)->groupInviteReceived(friendnumber, group_public_key);
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

    fileRecvQueue.append(ToxFile(filenumber, friendnumber, QByteArray(), filesize,
                    CString::toString(filename,filename_length).toUtf8(), ToxFile::RECEIVING));
    emit static_cast<Core*>(core)->fileReceiveRequested(fileRecvQueue.last());
}
void Core::onFileControlCallback(Tox*, int32_t friendnumber, uint8_t receive_send, uint8_t filenumber,
                                      uint8_t control_type, const uint8_t*, uint16_t, void *core)
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
    if (control_type == TOX_FILECONTROL_ACCEPT && receive_send == 1)
    {
        file->status = ToxFile::TRANSMITTING;
        emit static_cast<Core*>(core)->fileTransferAccepted(*file);
        qDebug() << "Core: File control callback, file accepted";
        file->sendFuture = QtConcurrent::run(sendAllFileData, static_cast<Core*>(core), file);
    }
    else if (receive_send == 1 && control_type == TOX_FILECONTROL_KILL)
    {
        qDebug() << QString("Core::onFileControlCallback: Transfer of file %1 cancelled by friend %2")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferCancelled(file->friendId, file->fileNum, ToxFile::SENDING);
        file->sendFuture.waitForFinished(); // Wait for sendAllFileData to return before deleting the ToxFile
        removeFileFromQueue(true, file->friendId, file->fileNum);
    }
    else if (receive_send == 0 && control_type == TOX_FILECONTROL_KILL)
    {
        qDebug() << QString("Core::onFileControlCallback: Transfer of file %1 cancelled by friend %2")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferCancelled(file->friendId, file->fileNum, ToxFile::RECEIVING);
        removeFileFromQueue(false, file->friendId, file->fileNum);
    }
    else if (receive_send == 0 && control_type == TOX_FILECONTROL_FINISHED)
    {
        qDebug() << QString("Core::onFileControlCallback: Reception of file %1 from %2 finished")
                    .arg(file->fileNum).arg(file->friendId);
        file->status = ToxFile::STOPPED;
        emit static_cast<Core*>(core)->fileTransferFinished(*file);
        removeFileFromQueue(false, file->friendId, file->fileNum);
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

    file->fileData.append((char*)data,length);
    file->bytesSent += length;
    //qDebug() << QString("Core::onFileDataCallback: received %1/%2 bytes").arg(file->fileData.size()).arg(file->filesize);
    emit static_cast<Core*>(core)->fileTransferInfo(file->friendId, file->fileNum,
                                            file->filesize, file->bytesSent, ToxFile::RECEIVING);
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
        emit friendAdded(friendId, userId);
    }
    saveConfiguration();
}

void Core::sendMessage(int friendId, const QString& message)
{
    CString cMessage(message);

    int messageId = tox_send_message(tox, friendId, cMessage.data(), cMessage.size());
    emit messageSentResult(friendId, message, messageId);
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
    CString cMessage(message);

    tox_group_message_send(tox, groupId, cMessage.data(), cMessage.size());
}

void Core::sendFile(int32_t friendId, QString Filename, QByteArray data)
{
    QByteArray fileName = Filename.toUtf8();
    int fileNum = tox_new_file_sender(tox, friendId, data.size(), (uint8_t*)fileName.data(), fileName.size());
    if (fileNum == -1)
    {
        qWarning() << "Core::sendFile: Can't create the Tox file sender";
        return;
    }
    qDebug() << QString("Core::sendFile: Created file sender %1 with friend %2").arg(fileNum).arg(friendId);

    fileSendQueue.append(ToxFile(fileNum, friendId, data, data.size(), fileName, ToxFile::SENDING));

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
        qWarning("Core::cancelFileSend: No such file in queue");
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
        qWarning() << "Core::pauseResumeFileRecv: File is stopped";
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
    file->sendFuture.waitForFinished(); // Wait until sendAllFileData returns before deleting
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

void Core::acceptFileRecvRequest(int friendId, int fileNum)
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

void Core::bootstrapDht()
{
    qDebug() << "Core: Bootstraping DHT";
    const Settings& s = Settings::getInstance();
    QList<Settings::DhtServer> dhtServerList = s.getDhtServerList();

    static int j = 0;
    int i=0;
    int listSize = dhtServerList.size();
    while (i<5)
    {
        const Settings::DhtServer& dhtServer = dhtServerList[j % listSize];
        if (tox_bootstrap_from_address(tox, dhtServer.address.toLatin1().data(),
            0, qToBigEndian(dhtServer.port), CUserId(dhtServer.userId).data()) == 1)
            qDebug() << QString("Core: Bootstraping from ")+dhtServer.name+QString(", addr ")+dhtServer.address.toLatin1().data()
                        +QString(", port ")+QString().setNum(dhtServer.port);
        else
            qDebug() << "Core: Error bootstraping from "+dhtServer.name;

        j++;
        i++;
    }
}

void Core::process()
{
    tox_do(tox);
#ifdef DEBUG
    //we want to see the debug messages immediately
    fflush(stdout);
#endif
    checkConnection();
    //int toxInterval = tox_do_interval(tox);
    //qDebug() << QString("Tox interval %1").arg(toxInterval);
    toxTimer->start(50);
}

void Core::checkConnection()
{
    static bool isConnected = false;

    if (tox_isconnected(tox) && !isConnected) {
        qDebug() << "Core: Connected to DHT";
        emit connected();
        isConnected = true;
    } else if (!tox_isconnected(tox) && isConnected) {
        qDebug() << "Core: Disconnected to DHT";
        emit disconnected();
        isConnected = false;
    }
}

void Core::loadConfiguration()
{
    QString path = Settings::getSettingsDirPath() + '/' + CONFIG_FILE_NAME;

    QFile configurationFile(path);

    if (!configurationFile.exists()) {
        qWarning() << "The Tox configuration file was not found";
        return;
    }

    if (!configurationFile.open(QIODevice::ReadOnly)) {
        qCritical() << "File " << path << " cannot be opened";
        return;
    }

    qint64 fileSize = configurationFile.size();
    if (fileSize > 0) {
        QByteArray data = configurationFile.readAll();
        tox_load(tox, reinterpret_cast<uint8_t *>(data.data()), data.size());
    }

    configurationFile.close();

    loadFriends();
}

void Core::saveConfiguration()
{
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

    path += '/' + CONFIG_FILE_NAME;
    QSaveFile configurationFile(path);
    if (!configurationFile.open(QIODevice::WriteOnly)) {
        qCritical() << "File " << path << " cannot be opened";
        return;
    }

    qDebug() << "Core: writing tox_save";
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
                        emit friendUsernameLoaded(ids[i], CString::toString(name, nameSize));
                    }
                    delete[] name;
                }

                const int statusMessageSize = tox_get_status_message_size(tox, ids[i]);
                if (statusMessageSize > 0) {
                    uint8_t *statusMessage = new uint8_t[statusMessageSize];
                    if (tox_get_status_message(tox, ids[i], statusMessage, statusMessageSize) == statusMessageSize) {
                        emit friendStatusMessageLoaded(ids[i], CString::toString(statusMessage, statusMessageSize));
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

int Core::joinGroupchat(int32_t friendnumber, const uint8_t* friend_group_public_key) const
{
    qDebug() << QString("Trying to join groupchat invite by friend %1").arg(friendnumber);
    return tox_join_groupchat(tox, friendnumber, friend_group_public_key);
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
    while (file->bytesSent < file->fileData.size())
    {
        if (file->status == ToxFile::PAUSED)
        {
            QThread::sleep(0);
            continue;
        }
        else if (file->status == ToxFile::STOPPED)
        {
            qWarning("Core::sendAllFileData: File is stopped");
            return;
        }
        emit core->fileTransferInfo(file->friendId, file->fileNum, file->filesize, file->bytesSent, ToxFile::SENDING);
        qApp->processEvents();
        int chunkSize = tox_file_data_size(core->tox, file->friendId);
        if (chunkSize == -1)
        {
            qWarning("Core::fileHeartbeat: Error getting preffered chunk size, aborting file send");
            file->status = ToxFile::STOPPED;
            emit core->fileTransferCancelled(file->friendId, file->fileNum, ToxFile::SENDING);
            tox_file_send_control(core->tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_KILL, nullptr, 0);
            removeFileFromQueue(true, file->friendId, file->fileNum);
            return;
        }
        chunkSize = std::min(chunkSize, file->fileData.size());
        QByteArray toSend = file->fileData.mid(file->bytesSent, chunkSize);
        if (tox_file_send_data(core->tox, file->friendId, file->fileNum, (uint8_t*)toSend.data(), toSend.size()) == -1)
        {
            //qWarning("Core::fileHeartbeat: Error sending data chunk");
            QThread::sleep(0);
            continue;
        }
        file->bytesSent += chunkSize;
        //qDebug() << QString("Core::fileHeartbeat: sent %1/%2 bytes").arg(file->bytesSent).arg(file->fileData.size());
    }
    qDebug("Core::fileHeartbeat: Transfer finished");
    tox_file_send_control(core->tox, file->friendId, 0, file->fileNum, TOX_FILECONTROL_FINISHED, nullptr, 0);
    file->status = ToxFile::STOPPED;
    emit core->fileTransferFinished(*file);
    removeFileFromQueue(true, file->friendId, file->fileNum);
}

void Core::onAvInvite(int32_t call_index, void* core)
{
    int friendId = toxav_get_peer_id(static_cast<Core*>(core)->toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV invite";
        return;
    }

    int transType = toxav_get_peer_transmission_type(static_cast<Core*>(core)->toxav, call_index, 0);
    if (transType == TypeVideo)
    {
        qDebug() << QString("Core: AV invite from %1 with video").arg(friendId);
        emit static_cast<Core*>(core)->avInvite(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV invite from %1 without video").arg(friendId);
        emit static_cast<Core*>(core)->avInvite(friendId, call_index, false);
    }
}

void Core::onAvStart(int32_t call_index, void* core)
{
    int friendId = toxav_get_peer_id(static_cast<Core*>(core)->toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV start";
        return;
    }

    int transType = toxav_get_peer_transmission_type(static_cast<Core*>(core)->toxav, call_index, 0);
    if (transType == TypeVideo)
    {
        qDebug() << QString("Core: AV start from %1 with video").arg(friendId);
        prepareCall(friendId, call_index, static_cast<Core*>(core)->toxav, true);
        emit static_cast<Core*>(core)->avStart(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV start from %1 without video").arg(friendId);
        prepareCall(friendId, call_index, static_cast<Core*>(core)->toxav, false);
        emit static_cast<Core*>(core)->avStart(friendId, call_index, false);
    }
}

void Core::onAvCancel(int32_t call_index, void* core)
{
    int friendId = toxav_get_peer_id(static_cast<Core*>(core)->toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV cancel";
        return;
    }
    qDebug() << QString("Core: AV cancel from %1").arg(friendId);

    emit static_cast<Core*>(core)->avCancel(friendId, call_index);
}

void Core::onAvReject(int32_t, void*)
{
    qDebug() << "Core: AV reject";
}

void Core::onAvEnd(int32_t call_index, void* core)
{
    int friendId = toxav_get_peer_id(static_cast<Core*>(core)->toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV end";
        return;
    }
    qDebug() << QString("Core: AV end from %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avEnd(friendId, call_index);
}

void Core::onAvRinging(int32_t call_index, void* core)
{
    int friendId = toxav_get_peer_id(static_cast<Core*>(core)->toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV ringing";
        return;
    }

    if (calls[call_index].videoEnabled)
    {
        qDebug() << QString("Core: AV ringing with %1 with video").arg(friendId);
        emit static_cast<Core*>(core)->avRinging(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV ringing with %1 without video").arg(friendId);
        emit static_cast<Core*>(core)->avRinging(friendId, call_index, false);
    }
}

void Core::onAvStarting(int32_t call_index, void* core)
{
    int friendId = toxav_get_peer_id(static_cast<Core*>(core)->toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV starting";
        return;
    }
    int transType = toxav_get_peer_transmission_type(static_cast<Core*>(core)->toxav, call_index, 0);
    if (transType == TypeVideo)
    {
        qDebug() << QString("Core: AV starting from %1 with video").arg(friendId);
        prepareCall(friendId, call_index, static_cast<Core*>(core)->toxav, true);
        emit static_cast<Core*>(core)->avStarting(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV starting from %1 without video").arg(friendId);
        prepareCall(friendId, call_index, static_cast<Core*>(core)->toxav, false);
        emit static_cast<Core*>(core)->avStarting(friendId, call_index, false);
    }
}

void Core::onAvEnding(int32_t call_index, void* core)
{
    int friendId = toxav_get_peer_id(static_cast<Core*>(core)->toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV ending";
        return;
    }
    qDebug() << QString("Core: AV ending from %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avEnding(friendId, call_index);
}

void Core::onAvError(int32_t, void*)
{
    qDebug() << "Core: AV error";
}

void Core::onAvRequestTimeout(int32_t call_index, void* core)
{
    int friendId = toxav_get_peer_id(static_cast<Core*>(core)->toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV request timeout";
        return;
    }
    qDebug() << QString("Core: AV request timeout with %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avRequestTimeout(friendId, call_index);
}

void Core::onAvPeerTimeout(int32_t call_index, void* core)
{
    int friendId = toxav_get_peer_id(static_cast<Core*>(core)->toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV peer timeout";
        return;
    }
    qDebug() << QString("Core: AV peer timeout with %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avPeerTimeout(friendId, call_index);
}

void Core::answerCall(int callId)
{    
    int friendId = toxav_get_peer_id(toxav, callId, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV answer peer ID";
        return;
    }
    int transType = toxav_get_peer_transmission_type(toxav, callId, 0);
    if (transType == TypeVideo)
    {
        qDebug() << QString("Core: answering call %1 with video").arg(callId);
        toxav_answer(toxav, callId, TypeVideo);
    }
    else
    {
        qDebug() << QString("Core: answering call %1 without video").arg(callId);
        toxav_answer(toxav, callId, TypeAudio);
    }
}

void Core::hangupCall(int callId)
{
    qDebug() << QString("Core: hanging up call %1").arg(callId);
    calls[callId].active = false;
    toxav_hangup(toxav, callId);
}

void Core::startCall(int friendId, bool video)
{
    int callId;
    if (video)
    {
        qDebug() << QString("Core: Starting new call with %1 with video").arg(friendId);
        toxav_call(toxav, &callId, friendId, TypeVideo, TOXAV_RINGING_TIME);
        calls[callId].videoEnabled=true;
    }
    else
    {
        qDebug() << QString("Core: Starting new call with %1 without video").arg(friendId);
        toxav_call(toxav, &callId, friendId, TypeAudio, TOXAV_RINGING_TIME);
        calls[callId].videoEnabled=false;
    }
}

void Core::cancelCall(int callId, int friendId)
{
    qDebug() << QString("Core: Cancelling call with %1").arg(friendId);
    calls[callId].active = false;
    toxav_cancel(toxav, callId, friendId, 0);
}

void Core::prepareCall(int friendId, int callId, ToxAv* toxav, bool videoEnabled)
{
    qDebug() << QString("Core: preparing call %1").arg(callId);
    calls[callId].callId = callId;
    calls[callId].friendId = friendId;
    calls[callId].codecSettings = av_DefaultSettings;
    calls[callId].codecSettings.max_video_width = TOXAV_VIDEO_WIDTH;
    calls[callId].codecSettings.max_video_height = TOXAV_VIDEO_HEIGHT;
    calls[callId].videoEnabled = videoEnabled;
    toxav_prepare_transmission(toxav, callId, &calls[callId].codecSettings, videoEnabled);

    // Prepare output
    QAudioFormat format;
    format.setSampleRate(calls[callId].codecSettings.audio_sample_rate);
    format.setChannelCount(calls[callId].codecSettings.audio_channels);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    if (!QAudioDeviceInfo::defaultOutputDevice().isFormatSupported(format))
    {
        calls[callId].audioOutput = nullptr;
        qWarning() << "Core: Raw audio format not supported by output backend, cannot play audio.";
    }
    else
    {
        calls[callId].audioOutput = new QAudioOutput(format);
        calls[callId].audioOutput->start(&calls[callId].audioBuffer);
        int error = calls[callId].audioOutput->error();
        if (error != QAudio::NoError)
        {
            qWarning() << QString("Core: Error %1 when starting audio output").arg(error);
        }
    }

    // Start input
    if (!QAudioDeviceInfo::defaultInputDevice().isFormatSupported(format))
    {
        calls[callId].audioInput = nullptr;
        qWarning() << "Default input format not supported, cannot record audio";
    }
    else
    {
        calls[callId].audioInput = new QAudioInput(format);
        calls[callId].audioInputDevice = calls[callId].audioInput->start();
    }

    // Go
    calls[callId].active = true;

    if (calls[callId].audioOutput != nullptr)
    {
        calls[callId].playAudioTimer->setInterval(5);
        calls[callId].playAudioTimer->setSingleShot(true);
        connect(calls[callId].playAudioTimer, &QTimer::timeout, [=](){playCallAudio(callId,toxav);});
        calls[callId].playAudioTimer->start();
    }

    if (calls[callId].audioInput != nullptr)
    {
        calls[callId].sendAudioTimer->setInterval(5);
        calls[callId].sendAudioTimer->setSingleShot(true);
        connect(calls[callId].sendAudioTimer, &QTimer::timeout, [=](){sendCallAudio(callId,toxav);});
        calls[callId].sendAudioTimer->start();
    }

    if (calls[callId].videoEnabled)
    {
        calls[callId].playVideoTimer->setInterval(50);
        calls[callId].playVideoTimer->setSingleShot(true);
        calls[callId].playVideoTimer->start();

        calls[callId].sendVideoTimer->setInterval(50);
        calls[callId].sendVideoTimer->setSingleShot(true);
        calls[callId].sendVideoTimer->start();

        Widget::getInstance()->getCamera()->suscribe();
    }
}

void Core::cleanupCall(int callId)
{
    qDebug() << QString("Core: cleaning up call %1").arg(callId);
    calls[callId].active = false;
    disconnect(calls[callId].playAudioTimer,0,0,0);
    disconnect(calls[callId].sendAudioTimer,0,0,0);
    calls[callId].playAudioTimer->stop();
    calls[callId].sendAudioTimer->stop();
    calls[callId].playVideoTimer->stop();
    calls[callId].sendVideoTimer->stop();
    if (calls[callId].audioOutput != nullptr)
    {
        delete calls[callId].audioOutput;
        calls[callId].audioOutput = nullptr;
    }
    if (calls[callId].audioInput != nullptr)
    {
        delete calls[callId].audioInput;
        calls[callId].audioInput = nullptr;
    }
    if (calls[callId].videoEnabled)
        Widget::getInstance()->getCamera()->unsuscribe();
    calls[callId].audioBuffer.clear();
}

void Core::playCallAudio(int callId, ToxAv* toxav)
{
    if (!calls[callId].active)
        return;
    int framesize = (calls[callId].codecSettings.audio_frame_duration * calls[callId].codecSettings.audio_sample_rate) / 1000;
    uint8_t buf[framesize*2];
    int len = toxav_recv_audio(toxav, callId, framesize, (int16_t*)buf);
    if (len < 0)
    {
        if (len == -3) // Not in call !
        {
            qWarning("Core: Trying to play audio in an inactive call!");
            return;
        }
        qDebug() << QString("Core::playCallAudio: Error receiving audio: %1").arg(len);
        calls[callId].playAudioTimer->start();
        return;
    }
    if (len == 0)
    {
        calls[callId].playAudioTimer->start();
        return;
    }
    //qDebug() << QString("Core: Received %1 bytes, %2 audio bytes free, %3 core buffer size")
    //            .arg(len*2).arg(calls[callId].audioOutput->bytesFree()).arg(calls[callId].audioBuffer.bufferSize());
    calls[callId].audioBuffer.write((char*)buf, len*2);
    int state = calls[callId].audioOutput->state();
    if (state != QAudio::ActiveState)
    {
        qDebug() << QString("Core: Audio state is %1").arg(state);
        if (state == 3 && calls[callId].audioBuffer.bufferSize() >= framesize*2)
            calls[callId].audioOutput->start(&calls[callId].audioBuffer);
    }
    int error = calls[callId].audioOutput->error();
    if (error != QAudio::NoError)
        qWarning() << QString("Core::playCallAudio: Error: %1").arg(error);

    calls[callId].playAudioTimer->start();
}

void Core::sendCallAudio(int callId, ToxAv* toxav)
{
    if (!calls[callId].active)
        return;
    int framesize = (calls[callId].codecSettings.audio_frame_duration * calls[callId].codecSettings.audio_sample_rate) / 1000;
    uint8_t buf[framesize*2], dest[framesize*2];
    int bytesReady = calls[callId].audioInput->bytesReady();
    if (bytesReady >= framesize*2)
    {
        calls[callId].audioInputDevice->read((char*)buf, framesize*2);
        int result = toxav_prepare_audio_frame(toxav, callId, dest, framesize*2, (int16_t*)buf, framesize);
        if (result < 0)
        {
            qWarning() << QString("Core: Unable to prepare audio frame, error %1").arg(result);
            calls[callId].sendAudioTimer->start();
            return;
        }
        result = toxav_send_audio(toxav, callId, dest, result);
        if (result < 0)
        {
            qWarning() << QString("Core: Unable to send audio frame, error %1").arg(result);
            calls[callId].sendAudioTimer->start();
            return;
        }
        calls[callId].sendAudioTimer->start();
    }
    else
        calls[callId].sendAudioTimer->start();
}

void Core::playCallVideo(int callId)
{
    if (!calls[callId].active || !calls[callId].videoEnabled)
        return;
    vpx_image_t *image;
    if(toxav_recv_video(toxav, callId, &image) == 0)
    {
        if (image)
        {
            emit videoFrameReceived(*image);
        }
        else
        {
            //qDebug() << "Core: Received null video frame\n";
        }
    }
    else
        qDebug() << "Core: Error receiving video frame\n";

    calls[callId].playVideoTimer->start();
}

void Core::sendCallVideo(int callId)
{
    if (!calls[callId].active || !calls[callId].videoEnabled)
        return;

    static const int bufsize = TOXAV_MAX_VIDEO_WIDTH * TOXAV_MAX_VIDEO_HEIGHT * 4;
    static uint8_t* videobuf = new uint8_t[bufsize];
    vpx_image frame = camera->getLastVPXImage();
    if (frame.w && frame.h)
    {
        int result;
        if((result = toxav_prepare_video_frame(toxav, callId, videobuf, bufsize, &frame)) < 0)
        {
            qDebug() << QString("Core: toxav_prepare_video_frame: error %1").arg(result);
            calls[callId].sendVideoTimer->start();
            return;
        }

        if((result = toxav_send_video(toxav, callId, (uint8_t*)videobuf, result)) < 0)
            qDebug() << QString("Core: toxav_send_video error: %1").arg(result);

    }
    else
        qDebug("Core::sendCallVideo: Invalid frame (bad camera ?)");

    calls[callId].sendVideoTimer->start();
}

void Core::groupInviteFriend(int friendId, int groupId)
{
    tox_invite_friend(tox, friendId, groupId);
}

void Core::createGroup()
{
    emit emptyGroupCreated(tox_add_groupchat(tox));
}
