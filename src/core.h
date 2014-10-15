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

#include <cstdint>
#include <QObject>

#include "corestructs.h"
#include "coreav.h"
#include "coredefines.h"

template <typename T> class QList;
class Camera;
class QTimer;
class QString;
class CString;

class Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(Camera* cam, QThread* coreThread, QString initialLoadPath);
    static Core* getInstance(); ///< Returns the global widget's Core instance
    ~Core();
    
    static const QString TOX_EXT;
    static const QString CONFIG_FILE_NAME;
    static QString sanitize(QString name);

    int getGroupNumberPeers(int groupId) const;
    QString getGroupPeerName(int groupId, int peerId) const;
    QList<QString> getGroupPeerNames(int groupId) const;
    QString getFriendAddress(int friendNumber) const;
    QString getFriendUsername(int friendNumber) const;
    int joinGroupchat(int32_t friendnumber, const uint8_t* friend_group_public_key,uint16_t length) const;
    void quitGroupChat(int groupId) const;
    void dispatchVideoFrame(vpx_image img) const;

    void saveConfiguration();
    void saveConfiguration(const QString& path);
    void switchConfiguration(QString profile);
    
    QString getIDString();
    
    QString getUsername();
    QString getStatusMessage();
    ToxID getSelfId();

    void increaseVideoBusyness();
    void decreaseVideoBusyness();

    bool anyActiveCalls();

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
    void setAvatar(uint8_t format, const QByteArray& data);

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

    void micMuteToggle(int callId);

signals:
    void connected();
    void disconnected();

    void friendRequestReceived(const QString& userId, const QString& message);
    void friendMessageReceived(int friendId, const QString& message, bool isAction);

    void friendAdded(int friendId, const QString& userId);

    void friendStatusChanged(int friendId, Status status);
    void friendStatusMessageChanged(int friendId, const QString& message);
    void friendUsernameChanged(int friendId, const QString& username);
    void friendTypingChanged(int friendId, bool isTyping);
    void friendAvatarChanged(int friendId, const QPixmap& pic);
    void friendAvatarRemoved(int friendId);

    void friendAddressGenerated(const QString& friendAddress);

    void friendRemoved(int friendId);

    void friendLastSeenChanged(int friendId, const QDateTime& dateTime);

    void emptyGroupCreated(int groupnumber);
    void groupInviteReceived(int friendnumber, const uint8_t *group_public_key,uint16_t length);
    void groupMessageReceived(int groupnumber, const QString& message, const QString& author);
    void groupNamelistChanged(int groupnumber, int peernumber, uint8_t change);

    void usernameSet(const QString& username);
    void statusMessageSet(const QString& message);
    void statusSet(Status status);
    void selfAvatarChanged(const QPixmap& pic);

    void messageSentResult(int friendId, const QString& message, int messageId);
    void groupSentResult(int groupId, const QString& message, int result);
    void actionSentResult(int friendId, const QString& action, int success);

    void failedToAddFriend(const QString& userId);
    void failedToRemoveFriend(int friendId);
    void failedToSetUsername(const QString& username);
    void failedToSetStatusMessage(const QString& message);
    void failedToSetStatus(Status status);
    void failedToSetTyping(bool typing);

    void failedToStart();
    void badProxy();

    void fileSendStarted(ToxFile file);
    void fileReceiveRequested(ToxFile file);
    void fileTransferAccepted(ToxFile file);
    void fileTransferCancelled(int FriendId, int FileNum, ToxFile::FileDirection direction);
    void fileTransferFinished(ToxFile file);
    void fileUploadFinished(const QString& path);
    void fileDownloadFinished(const QString& path);
    void fileTransferPaused(int FriendId, int FileNum, ToxFile::FileDirection direction);
    void fileTransferInfo(int FriendId, int FileNum, int64_t Filesize, int64_t BytesSent, ToxFile::FileDirection direction);
    void fileTransferRemotePausedUnpaused(ToxFile file, bool paused);
    void fileTransferBrokenUnbroken(ToxFile file, bool broken);

    void fileSendFailed(int FriendId, const QString& fname);

    void avInvite(int friendId, int callIndex, bool video);
    void avStart(int friendId, int callIndex, bool video);
    void avCancel(int friendId, int callIndex);
    void avEnd(int friendId, int callIndex);
    void avRinging(int friendId, int callIndex, bool video);
    void avStarting(int friendId, int callIndex, bool video);
    void avEnding(int friendId, int callIndex);
    void avRequestTimeout(int friendId, int callIndex);
    void avPeerTimeout(int friendId, int callIndex);
    void avMediaChange(int friendId, int callIndex, bool videoEnabled);

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
    static void onGroupInvite(Tox *tox, int friendnumber, const uint8_t *group_public_key, uint16_t length,void *userdata);
    static void onGroupMessage(Tox *tox, int groupnumber, int friendgroupnumber, const uint8_t * message, uint16_t length, void *userdata);
    static void onGroupNamelistChange(Tox *tox, int groupnumber, int peernumber, uint8_t change, void *userdata);
    static void onFileSendRequestCallback(Tox *tox, int32_t friendnumber, uint8_t filenumber, uint64_t filesize,
                                          const uint8_t *filename, uint16_t filename_length, void *userdata);
    static void onFileControlCallback(Tox *tox, int32_t friendnumber, uint8_t receive_send, uint8_t filenumber,
                                      uint8_t control_type, const uint8_t *data, uint16_t length, void *core);
    static void onFileDataCallback(Tox *tox, int32_t friendnumber, uint8_t filenumber, const uint8_t *data, uint16_t length, void *userdata);
    static void onAvatarInfoCallback(Tox* tox, int32_t friendnumber, uint8_t format, uint8_t *hash, void *userdata);
    static void onAvatarDataCallback(Tox* tox, int32_t friendnumber, uint8_t format, uint8_t *hash, uint8_t *data, uint32_t datalen, void *userdata);

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
    static void onAvMediaChange(void *toxav, int32_t call_index, void* core);

    static void prepareCall(int friendId, int callId, ToxAv *toxav, bool videoEnabled);
    static void cleanupCall(int callId);
    static void playCallAudio(ToxAv *toxav, int32_t callId, int16_t *data, int samples, void *user_data); // Callback
    static void sendCallAudio(int callId, ToxAv* toxav);
    static void playAudioBuffer(int callId, int16_t *data, int samples, unsigned channels, int sampleRate);
    static void playCallVideo(ToxAv* toxav, int32_t callId, vpx_image_t* img, void *user_data);
    void sendCallVideo(int callId);

    bool checkConnection();

    bool loadConfiguration(QString path); // Returns false for a critical error, true otherwise
    void make_tox();
    void loadFriends();

    static void sendAllFileData(Core* core, ToxFile* file);
    static void removeFileFromQueue(bool sendQueue, int friendId, int fileId);

    void checkLastOnline(int friendId);

    QList<CString> splitMessage(const QString &message);

private slots:
     void onFileTransferFinished(ToxFile file);

private:
    Tox* tox;
    ToxAv* toxav;
    QTimer *toxTimer, *fileTimer; //, *saveTimer;
    Camera* camera;
    QString loadPath; // meaningless after start() is called
    QList<DhtServer> dhtServerList;
    int dhtServerId;
    static QList<ToxFile> fileSendQueue, fileRecvQueue;
    static ToxCall calls[];

    static const int videobufsize;
    static uint8_t* videobuf;
    static int videoBusyness; // Used to know when to drop frames

    static ALCdevice* alOutDev, *alInDev;
    static ALCcontext* alContext;
public:
    static ALuint alMainSource;
};

#endif // CORE_HPP

