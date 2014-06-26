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

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>
#include <QtEndian>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

#include "settings.h"

#define GROUPCHAT_MAX_SIZE 32
#define TOX_SAVE_INTERVAL 30*1000
#define TOX_FILE_INTERVAL 20
#define TOX_BOOTSTRAP_INTERVAL 10*1000

const QString Core::CONFIG_FILE_NAME = "tox_save";
QList<ToxFile> Core::fileSendQueue;
QList<ToxFile> Core::fileRecvQueue;

Core::Core() :
    tox(nullptr)
{
    toxTimer = new QTimer(this);
    toxTimer->setSingleShot(true);
    saveTimer = new QTimer(this);
    saveTimer->start(TOX_SAVE_INTERVAL);
    //fileTimer = new QTimer(this);
    //fileTimer->start(TOX_FILE_INTERVAL);
    bootstrapTimer = new QTimer(this);
    bootstrapTimer->start(TOX_BOOTSTRAP_INTERVAL);
    connect(toxTimer, &QTimer::timeout, this, &Core::process);
    connect(saveTimer, &QTimer::timeout, this, &Core::saveConfiguration);
    //connect(fileTimer, &QTimer::timeout, this, &Core::fileHeartbeat);
    connect(bootstrapTimer, &QTimer::timeout, this, &Core::onBootstrapTimer);
    connect(&Settings::getInstance(), &Settings::dhtServerListChanged, this, &Core::bootstrapDht);
}

Core::~Core()
{
    if (tox) {
        saveConfiguration();
        tox_kill(tox);
    }
}

void Core::onBootstrapTimer()
{
    if(!tox_isconnected(tox))
        bootstrapDht();
}

void Core::onFriendRequest(Tox*/* tox*/, const uint8_t* cUserId, const uint8_t* cMessage, uint16_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendRequestReceived(CUserId::toString(cUserId), CString::toString(cMessage, cMessageSize));
}

void Core::onFriendMessage(Tox*/* tox*/, int friendId, uint8_t* cMessage, uint16_t cMessageSize, void* core)
{
    emit static_cast<Core*>(core)->friendMessageReceived(friendId, CString::toString(cMessage, cMessageSize));
}

void Core::onFriendNameChange(Tox*/* tox*/, int friendId, uint8_t* cName, uint16_t cNameSize, void* core)
{
    emit static_cast<Core*>(core)->friendUsernameChanged(friendId, CString::toString(cName, cNameSize));
}

void Core::onFriendTypingChange(Tox*/* tox*/, int friendId, uint8_t isTyping, void *core)
{
    emit static_cast<Core*>(core)->friendTypingChanged(friendId, isTyping ? true : false);
}

void Core::onStatusMessageChanged(Tox*/* tox*/, int friendId, uint8_t* cMessage, uint16_t cMessageSize, void* core)
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

void Core::onAction(Tox*/* tox*/, int friendId, uint8_t *cMessage, uint16_t cMessageSize, void *core)
{
    emit static_cast<Core*>(core)->actionReceived(friendId, CString::toString(cMessage, cMessageSize));
}

void Core::onGroupInvite(Tox*, int friendnumber, uint8_t *group_public_key, void *core)
{
    qDebug() << QString("Core: Group invite by %1").arg(friendnumber);
    emit static_cast<Core*>(core)->groupInviteReceived(friendnumber, group_public_key);
}

void Core::onGroupMessage(Tox*, int groupnumber, int friendgroupnumber, uint8_t * message, uint16_t length, void *core)
{
    emit static_cast<Core*>(core)->groupMessageReceived(groupnumber, friendgroupnumber, CString::toString(message, length));
}

void Core::onGroupNamelistChange(Tox*, int groupnumber, int peernumber, uint8_t change, void *core)
{
    qDebug() << QString("Core: Group namelist change %1:%2 %3").arg(groupnumber).arg(peernumber).arg(change);
    emit static_cast<Core*>(core)->groupNamelistChanged(groupnumber, peernumber, change);
}

void Core::onFileSendRequestCallback(Tox*, int32_t friendnumber, uint8_t filenumber, uint64_t filesize,
                                          uint8_t *filename, uint16_t filename_length, void *core)
{
    qDebug() << QString("Core: Received file request %1 with friend %2").arg(filenumber).arg(friendnumber);

    fileRecvQueue.append(ToxFile(filenumber, friendnumber, QByteArray(), filesize,
                    CString::toString(filename,filename_length).toUtf8(), ToxFile::RECEIVING));
    emit static_cast<Core*>(core)->fileReceiveRequested(fileRecvQueue.last());
}
void Core::onFileControlCallback(Tox*, int32_t friendnumber, uint8_t receive_send, uint8_t filenumber,
                                      uint8_t control_type, uint8_t*, uint16_t, void *core)
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

void Core::onFileDataCallback(Tox*, int32_t friendnumber, uint8_t filenumber, uint8_t *data, uint16_t length, void *core)
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
        emit usernameSet(username);
    }
}

void Core::setStatusMessage(const QString& message)
{
    CString cMessage(message);

    if (tox_set_status_message(tox, cMessage.data(), cMessage.size()) == -1) {
        emit failedToSetStatusMessage(message);
    } else {
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
                        +QString(", port ")+QString().setNum(qToBigEndian(dhtServer.port));
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
    toxTimer->start(tox_do_interval(tox));
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

void Core::start()
{
    tox = tox_new(1);
    if (tox == nullptr) {
        qCritical() << "Core failed to start";
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

    uint8_t friendAddress[TOX_FRIEND_ADDRESS_SIZE];
    tox_get_address(tox, friendAddress);

    emit friendAddressGenerated(CFriendAddress::toString(friendAddress));

    CString cUsername(Settings::getInstance().getUsername());
    tox_set_name(tox, cUsername.data(), cUsername.size());

    CString cStatusMessage(Settings::getInstance().getStatusMessage());
    tox_set_status_message(tox, cStatusMessage.data(), cStatusMessage.size());

    bootstrapDht();

    toxTimer->start(tox_do_interval(tox));
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

int Core::joinGroupchat(int32_t friendnumber, uint8_t* friend_group_public_key) const
{
    return tox_join_groupchat(tox, friendnumber, friend_group_public_key);
}

// CData

Core::CData::CData(const QString &data, uint16_t byteSize)
{
    cData = new uint8_t[byteSize];
    cDataSize = fromString(data, cData);
}

Core::CData::~CData()
{
    delete[] cData;
}

uint8_t* Core::CData::data()
{
    return cData;
}

uint16_t Core::CData::size()
{
    return cDataSize;
}

QString Core::CData::toString(const uint8_t *cData, const uint16_t cDataSize)
{
    return QString(QByteArray(reinterpret_cast<const char*>(cData), cDataSize).toHex()).toUpper();
}

uint16_t Core::CData::fromString(const QString& data, uint8_t* cData)
{
    QByteArray arr = QByteArray::fromHex(data.toLower().toLatin1());
    memcpy(cData, reinterpret_cast<uint8_t*>(arr.data()), arr.size());
    return arr.size();
}


// CUserId

Core::CUserId::CUserId(const QString &userId) :
    CData(userId, SIZE)
{
    // intentionally left empty
}

QString Core::CUserId::toString(const uint8_t* cUserId)
{
    return CData::toString(cUserId, SIZE);
}


// CFriendAddress

Core::CFriendAddress::CFriendAddress(const QString &friendAddress) :
    CData(friendAddress, SIZE)
{
    // intentionally left empty
}

QString Core::CFriendAddress::toString(const uint8_t *cFriendAddress)
{
    return CData::toString(cFriendAddress, SIZE);
}


// CString

Core::CString::CString(const QString& string)
{
    cString = new uint8_t[string.length() * MAX_SIZE_OF_UTF8_ENCODED_CHARACTER]();
    cStringSize = fromString(string, cString);
}

Core::CString::~CString()
{
    delete[] cString;
}

uint8_t* Core::CString::data()
{
    return cString;
}

uint16_t Core::CString::size()
{
    return cStringSize;
}

QString Core::CString::toString(const uint8_t* cString, uint16_t cStringSize)
{
    return QString::fromUtf8(reinterpret_cast<const char*>(cString), cStringSize);
}

uint16_t Core::CString::fromString(const QString& string, uint8_t* cString)
{
    QByteArray byteArray = QByteArray(string.toUtf8());
    memcpy(cString, reinterpret_cast<uint8_t*>(byteArray.data()), byteArray.size());
    return byteArray.size();
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
