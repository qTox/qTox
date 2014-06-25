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

#ifndef CORE_HPP
#define CORE_HPP

#include "status.h"

#include <tox/tox.h>

#include <cstdint>
#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <QString>
#include <QList>
#include <QByteArray>

#define GROUPCHAT_MAX_SIZE 32
#define TOX_SAVE_INTERVAL 30*1000
#define TOX_FILE_INTERVAL 50

struct DhtServer
{
    QString name;
    QString userId;
    QString address;
    int port;
};

struct ToxFile
{
    enum FileStatus
    {
        STOPPED,
        PAUSED,
        TRANSMITTING
    };

    enum FileDirection : bool
    {
        SENDING,
        RECEIVING
    };

    ToxFile(int FileNum, int FriendId, QByteArray FileData, QByteArray FileName, FileDirection Direction)
        : fileNum(FileNum), friendId(FriendId), fileData{FileData},
          fileName{FileName}, bytesSent{0}, status{STOPPED}, direction{Direction} {}

    int fileNum;
    int friendId;
    QByteArray fileData;
    QByteArray fileName;
    long long bytesSent;
    FileStatus status;
    FileDirection direction;
};

class Core : public QObject
{
    Q_OBJECT
public:
    explicit Core();
    ~Core();

private:
    static void onFriendRequest(Tox* tox, const uint8_t* cUserId, const uint8_t* cMessage, uint16_t cMessageSize, void* core);
    static void onFriendMessage(Tox* tox, int friendId, uint8_t* cMessage, uint16_t cMessageSize, void* core);
    static void onFriendNameChange(Tox* tox, int friendId, uint8_t* cName, uint16_t cNameSize, void* core);
    static void onFriendTypingChange(Tox* tox, int friendId, uint8_t isTyping, void* core);
    static void onStatusMessageChanged(Tox* tox, int friendId, uint8_t* cMessage, uint16_t cMessageSize, void* core);
    static void onUserStatusChanged(Tox* tox, int friendId, uint8_t userstatus, void* core);
    static void onConnectionStatusChanged(Tox* tox, int friendId, uint8_t status, void* core);
    static void onAction(Tox* tox, int friendId, uint8_t* cMessage, uint16_t cMessageSize, void* core);
    static void onGroupInvite(Tox *tox, int friendnumber, uint8_t *group_public_key, void *userdata);
    static void onGroupMessage(Tox *tox, int groupnumber, int friendgroupnumber, uint8_t * message, uint16_t length, void *userdata);
    static void onGroupNamelistChange(Tox *tox, int groupnumber, int peernumber, uint8_t change, void *userdata);
    static void onFileSendRequestCallback(Tox *tox, int32_t friendnumber, uint8_t filenumber, uint64_t filesize,
                                          uint8_t *filename, uint16_t filename_length, void *userdata);
    static void onFileControlCallback(Tox *tox, int32_t friendnumber, uint8_t receive_send, uint8_t filenumber,
                                      uint8_t control_type, uint8_t *data, uint16_t length, void *core);
    static void onFileDataCallback(Tox *tox, int32_t friendnumber, uint8_t filenumber, uint8_t *data, uint16_t length, void *userdata);

    void checkConnection();

    void loadConfiguration();
    void saveConfiguration();
    void loadFriends();

    void checkLastOnline(int friendId);

    Tox* tox;
    QTimer *toxTimer, *saveTimer, *fileTimer;
    QList<DhtServer> dhtServerList;
    int dhtServerId;
    static QList<ToxFile> fileSendQueue;

    static const QString CONFIG_FILE_NAME;

    class CData
    {
    public:
        uint8_t* data();
        uint16_t size();

    protected:
        explicit CData(const QString& data, uint16_t byteSize);
        virtual ~CData();

        static QString toString(const uint8_t* cData, const uint16_t cDataSize);

    private:
        uint8_t* cData;
        uint16_t cDataSize;

        static uint16_t fromString(const QString& userId, uint8_t* cData);
    };

    class CUserId : public CData
    {
    public:
        explicit CUserId(const QString& userId);

        static QString toString(const uint8_t *cUserId);

    private:
        static const uint16_t SIZE = TOX_CLIENT_ID_SIZE;

    };

    class CFriendAddress : public CData
    {
    public:
        explicit CFriendAddress(const QString& friendAddress);

        static QString toString(const uint8_t* cFriendAddress);

    private:
        static const uint16_t SIZE = TOX_FRIEND_ADDRESS_SIZE;

    };

    class CString
    {
    public:
        explicit CString(const QString& string);
        ~CString();

        uint8_t* data();
        uint16_t size();

        static QString toString(const uint8_t* cMessage, const uint16_t cMessageSize);

    private:
        const static int MAX_SIZE_OF_UTF8_ENCODED_CHARACTER = 4;

        uint8_t* cString;
        uint16_t cStringSize;

        static uint16_t fromString(const QString& message, uint8_t* cMessage);
    };

public:
    int getGroupNumberPeers(int groupId) const;
    QString getGroupPeerName(int groupId, int peerId) const;
    QList<QString> getGroupPeerNames(int groupId) const;
    int joinGroupchat(int32_t friendnumber, uint8_t* friend_group_public_key) const;
    void quitGroupChat(int groupId) const;

public slots:
    void start();

    void acceptFriendRequest(const QString& userId);
    void requestFriendship(const QString& friendAddress, const QString& message);

    void removeFriend(int friendId);
    void removeGroup(int groupId);

    void sendMessage(int friendId, const QString& message);
    void sendGroupMessage(int groupId, const QString& message);
    void sendAction(int friendId, const QString& action);
    void sendTyping(int friendId, bool typing);

    void sendFile(int32_t friendId, QString Filename, QByteArray data);

    void setUsername(const QString& username);
    void setStatusMessage(const QString& message);
    void setStatus(Status status);

    void process();
    void fileHeartbeat();

    void bootstrapDht();

signals:
    void connected();
    void disconnected();

    void friendRequestReceived(const QString& userId, const QString& message);
    void friendMessageReceived(int friendId, const QString& message);

    void friendAdded(int friendId, const QString& userId);

    void friendStatusChanged(int friendId, Status status);
    void friendStatusMessageChanged(int friendId, const QString& message);
    void friendUsernameChanged(int friendId, const QString& username);
    void friendTypingChanged(int friendId, bool isTyping);

    void friendStatusMessageLoaded(int friendId, const QString& message);
    void friendUsernameLoaded(int friendId, const QString& username);

    void friendAddressGenerated(const QString& friendAddress);

    void friendRemoved(int friendId);

    void friendLastSeenChanged(int friendId, const QDateTime& dateTime);

    void groupInviteReceived(int friendnumber, uint8_t *group_public_key);
    void groupMessageReceived(int groupnumber, int friendgroupnumber, const QString& message);
    void groupNamelistChanged(int groupnumber, int peernumber, uint8_t change);

    void usernameSet(const QString& username);
    void statusMessageSet(const QString& message);
    void statusSet(Status status);

    void messageSentResult(int friendId, const QString& message, int messageId);
    void actionSentResult(int friendId, const QString& action, int success);

    void failedToAddFriend(const QString& userId);
    void failedToRemoveFriend(int friendId);
    void failedToSetUsername(const QString& username);
    void failedToSetStatusMessage(const QString& message);
    void failedToSetStatus(Status status);
    void failedToSetTyping(bool typing);

    void actionReceived(int friendId, const QString& acionMessage);

    void failedToStart();

    void fileSendStarted(ToxFile* file);
    void fileTransferAccepted(ToxFile* file);
    void fileTransferCancelled(ToxFile* file);
    void fileTransferFinished(ToxFile* file);
    void fileTransferPaused(ToxFile* file);
    void fileTransferInfo(ToxFile* file);
};

#endif // CORE_HPP

