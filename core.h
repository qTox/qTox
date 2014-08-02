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

#include "audiobuffer.h"

#include <tox/tox.h>
#include <tox/toxav.h>

#include <cstdint>
#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <QString>
#include <QFile>
#include <QList>
#include <QByteArray>
#include <QFuture>
#include <QBuffer>
#include <QAudioOutput>
#include <QAudioInput>

#define TOXAV_MAX_CALLS 16
#define GROUPCHAT_MAX_SIZE 32
#define TOX_SAVE_INTERVAL 30*1000
#define TOX_FILE_INTERVAL 20
#define TOX_BOOTSTRAP_INTERVAL 10*1000
#define TOXAV_RINGING_TIME 15

// TODO: Put that in the settings
#define TOXAV_MAX_VIDEO_WIDTH 1600
#define TOXAV_MAX_VIDEO_HEIGHT 1200

class Camera;

enum class Status : int {Online = 0, Away, Busy, Offline};

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

    ToxFile()=default;
    ToxFile(int FileNum, int FriendId, QByteArray FileName, QString FilePath, FileDirection Direction)
        : fileNum(FileNum), friendId(FriendId), fileName{FileName}, filePath{FilePath}, file{new QFile(filePath)},
        bytesSent{0}, filesize{0}, status{STOPPED}, direction{Direction} {}
    ~ToxFile(){}
    void setFilePath(QString path) {filePath=path; file->setFileName(path);}
    bool open(bool write) {return write?file->open(QIODevice::ReadWrite):file->open(QIODevice::ReadOnly);}

    int fileNum;
    int friendId;
    QByteArray fileName;
    QString filePath;
    QFile* file;
    long long bytesSent;
    long long filesize;
    FileStatus status;
    FileDirection direction;
    QFuture<void> sendFuture;
};

struct ToxCall
{
public:
    AudioBuffer audioBuffer;
    QAudioOutput* audioOutput;
    QAudioInput* audioInput;
    QIODevice* audioInputDevice;
    ToxAvCSettings codecSettings;
    QTimer *sendAudioTimer, *sendVideoTimer;
    int callId;
    int friendId;
    bool videoEnabled;
    bool active;
};

class Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(Camera* cam, QThread* coreThread);
    ~Core();

    int getGroupNumberPeers(int groupId) const;
    QString getGroupPeerName(int groupId, int peerId) const;
    QList<QString> getGroupPeerNames(int groupId) const;
    int joinGroupchat(int32_t friendnumber, const uint8_t* friend_group_public_key) const;
    void quitGroupChat(int groupId) const;
    void dispatchVideoFrame(vpx_image img) const;

    void saveConfiguration();
    
    QString getUsername();
    QString getStatusMessage();

    void increaseVideoBusyness();
    void decreaseVideoBusyness();

public slots:
    void start();
    void process();
    void bootstrapDht();

    void acceptFriendRequest(const QString& userId);
    void requestFriendship(const QString& friendAddress, const QString& message);
    void groupInviteFriend(int friendId, int groupId);
    void createGroup();

    void removeFriend(int friendId);
    void removeGroup(int groupId);

    void setStatus(Status status);
    void setUsername(const QString& username);
    void setStatusMessage(const QString& message);

    void sendMessage(int friendId, const QString& message);
    void sendGroupMessage(int groupId, const QString& message);
    void sendAction(int friendId, const QString& action);
    void sendTyping(int friendId, bool typing);

    void sendFile(int32_t friendId, QString Filename, QString FilePath, long long filesize);
    void cancelFileSend(int friendId, int fileNum);
    void cancelFileRecv(int friendId, int fileNum);
    void rejectFileRecvRequest(int friendId, int fileNum);
    void acceptFileRecvRequest(int friendId, int fileNum, QString path);
    void pauseResumeFileSend(int friendId, int fileNum);
    void pauseResumeFileRecv(int friendId, int fileNum);

    void answerCall(int callId);
    void hangupCall(int callId);
    void startCall(int friendId, bool video=false);
    void cancelCall(int callId, int friendId);

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

    void emptyGroupCreated(int groupnumber);
    void groupInviteReceived(int friendnumber, const uint8_t *group_public_key);
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

    void fileSendStarted(ToxFile file);
    void fileReceiveRequested(ToxFile file);
    void fileTransferAccepted(ToxFile file);
    void fileTransferCancelled(int FriendId, int FileNum, ToxFile::FileDirection direction);
    void fileTransferFinished(ToxFile file);
    void fileUploadFinished(const QString& path);
    void fileDownloadFinished(const QString& path);
    void fileTransferPaused(int FriendId, int FileNum, ToxFile::FileDirection direction);
    void fileTransferInfo(int FriendId, int FileNum, int64_t Filesize, int64_t BytesSent, ToxFile::FileDirection direction);

    void avInvite(int friendId, int callIndex, bool video);
    void avStart(int friendId, int callIndex, bool video);
    void avCancel(int friendId, int callIndex);
    void avEnd(int friendId, int callIndex);
    void avRinging(int friendId, int callIndex, bool video);
    void avStarting(int friendId, int callIndex, bool video);
    void avEnding(int friendId, int callIndex);
    void avRequestTimeout(int friendId, int callIndex);
    void avPeerTimeout(int friendId, int callIndex);
    void avMediaChange(int friendId, int callIndex);

    void videoFrameReceived(vpx_image* frame);

private:
    static void onFriendRequest(Tox* tox, const uint8_t* cUserId, const uint8_t* cMessage, uint16_t cMessageSize, void* core);
    static void onFriendMessage(Tox* tox, int friendId, const uint8_t* cMessage, uint16_t cMessageSize, void* core);
    static void onFriendNameChange(Tox* tox, int friendId, const uint8_t* cName, uint16_t cNameSize, void* core);
    static void onFriendTypingChange(Tox* tox, int friendId, uint8_t isTyping, void* core);
    static void onStatusMessageChanged(Tox* tox, int friendId, const uint8_t* cMessage, uint16_t cMessageSize, void* core);
    static void onUserStatusChanged(Tox* tox, int friendId, uint8_t userstatus, void* core);
    static void onConnectionStatusChanged(Tox* tox, int friendId, uint8_t status, void* core);
    static void onAction(Tox* tox, int friendId, const uint8_t* cMessage, uint16_t cMessageSize, void* core);
    static void onGroupInvite(Tox *tox, int friendnumber, const uint8_t *group_public_key, void *userdata);
    static void onGroupMessage(Tox *tox, int groupnumber, int friendgroupnumber, const uint8_t * message, uint16_t length, void *userdata);
    static void onGroupNamelistChange(Tox *tox, int groupnumber, int peernumber, uint8_t change, void *userdata);
    static void onFileSendRequestCallback(Tox *tox, int32_t friendnumber, uint8_t filenumber, uint64_t filesize,
                                          const uint8_t *filename, uint16_t filename_length, void *userdata);
    static void onFileControlCallback(Tox *tox, int32_t friendnumber, uint8_t receive_send, uint8_t filenumber,
                                      uint8_t control_type, const uint8_t *data, uint16_t length, void *core);
    static void onFileDataCallback(Tox *tox, int32_t friendnumber, uint8_t filenumber, const uint8_t *data, uint16_t length, void *userdata);

    static void onAvInvite(void* toxav, int32_t call_index, void* core);
    static void onAvStart(void* toxav, int32_t call_index, void* core);
    static void onAvCancel(void* toxav, int32_t call_index, void* core);
    static void onAvReject(void* toxav, int32_t call_index, void* core);
    static void onAvEnd(void* toxav, int32_t call_index, void* core);
    static void onAvRinging(void* toxav, int32_t call_index, void* core);
    static void onAvStarting(void* toxav, int32_t call_index, void* core);
    static void onAvEnding(void* toxav, int32_t call_index, void* core);
    static void onAvRequestTimeout(void* toxav, int32_t call_index, void* core);
    static void onAvPeerTimeout(void* toxav, int32_t call_index, void* core);
    static void onAvMediaChange(void* toxav, int32_t call_index, void* core);

    static void prepareCall(int friendId, int callId, ToxAv *toxav, bool videoEnabled);
    static void cleanupCall(int callId);
    static void playCallAudio(ToxAv *toxav, int32_t callId, int16_t *data, int length, void *user_data); // Callback
    static void sendCallAudio(int callId, ToxAv* toxav);
    static void playCallVideo(ToxAv* toxav, int32_t callId, vpx_image_t* img, void *user_data);
    void sendCallVideo(int callId);

    void checkConnection();
    void onBootstrapTimer();

    void loadConfiguration();
    void loadFriends();
    static void sendAllFileData(Core* core, ToxFile* file);

    static void removeFileFromQueue(bool sendQueue, int friendId, int fileId);

    void checkLastOnline(int friendId);

private slots:
     void onFileTransferFinished(ToxFile file);

private:
    Tox* tox;
    ToxAv* toxav;
    QTimer *toxTimer, *fileTimer, *bootstrapTimer; //, *saveTimer;
    Camera* camera;
    QList<DhtServer> dhtServerList;
    int dhtServerId;
    static QList<ToxFile> fileSendQueue, fileRecvQueue;
    static ToxCall calls[TOXAV_MAX_CALLS];

    static const QString CONFIG_FILE_NAME;
    static const int videobufsize;
    static uint8_t* videobuf;
    static int videoBusyness; // Used to know when to drop frames
};

#endif // CORE_HPP

