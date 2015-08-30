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

#ifndef CORE_HPP
#define CORE_HPP

#include <cstdint>
#include <QObject>
#include <QMutex>

#include <tox/tox.h>
#include <tox/toxencryptsave.h>

#include "corestructs.h"
#include "coreav.h"
#include "coredefines.h"
#include "toxid.h"

class Profile;
template <typename T> class QList;
class QTimer;
class QString;
class CString;
class VideoSource;
class VideoFrame;
#ifdef QTOX_FILTER_AUDIO
class AudioFilterer;
#endif

class Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(QThread* coreThread, Profile& profile);
    static Core* getInstance(); ///< Returns the global widget's Core instance
    ~Core();

    static const QString TOX_EXT;
    static const QString CONFIG_FILE_NAME;
    static QString sanitize(QString name);
    static QList<CString> splitMessage(const QString &message, int maxLen);

    static QByteArray getSaltFromFile(QString filename);

    QString getPeerName(const ToxId& id) const;

    QVector<uint32_t> getFriendList() const; ///< Returns the list of friendIds in our friendlist, an empty list on error
    int getGroupNumberPeers(int groupId) const; ///< Return the number of peers in the group chat on success, or -1 on failure
    QString getGroupPeerName(int groupId, int peerId) const; ///< Get the name of a peer of a group
    ToxId getGroupPeerToxId(int groupId, int peerId) const; ///< Get the public key of a peer of a group
    QList<QString> getGroupPeerNames(int groupId) const; ///< Get the names of the peers of a group
    QString getFriendAddress(uint32_t friendId) const; ///< Get the full address if known, or public key of a friend
    QString getFriendUsername(uint32_t friendId) const; ///< Get the username of a friend
    bool isFriendOnline(uint32_t friendId) const; ///< Check if a friend is online. Unknown friends are considered offline.
    bool hasFriendWithAddress(const QString &addr) const; ///< Check if we have a friend by address
    bool hasFriendWithPublicKey(const QString &pubkey) const; ///< Check if we have a friend by public key
    int joinGroupchat(int32_t friendId, uint8_t type, const uint8_t* pubkey,uint16_t length) const; ///< Accept a groupchat invite
    void quitGroupChat(int groupId) const; ///< Quit a groupchat

    QString getIDString() const; ///< Get the 12 first characters of our Tox ID

    QString getUsername() const; ///< Returns our username, or an empty string on failure
    QString getStatusMessage() const; ///< Returns our status message, or an empty string on failure
    ToxId getSelfId() const; ///< Returns our Tox ID
    QPair<QByteArray, QByteArray> getKeypair() const; ///< Returns our public and private keys

    static std::unique_ptr<TOX_PASS_KEY> createPasskey(const QString &password, uint8_t* salt = nullptr);
    static QByteArray encryptData(const QByteArray& data, const TOX_PASS_KEY& encryptionKey);
    static QByteArray encryptData(const QByteArray& data); ///< Uses the default profile's key
    static QByteArray decryptData(const QByteArray& data, const TOX_PASS_KEY &encryptionKey);
    static QByteArray decryptData(const QByteArray& data); ///< Uses the default profile's key

    VideoSource* getVideoSourceFromCall(int callNumber); ///< Get a call's video source

    static bool anyActiveCalls(); ///< true is any calls are currently active (note: a call about to start is not yet active)
    bool isReady(); ///< Most of the API shouldn't be used until Core is ready, call start() first

    void resetCallSources(); ///< Forces to regenerate each call's audio sources

public slots:
    void start(); ///< Initializes the core, must be called before anything else
    void reset(); ///< Reinitialized the core. Must be called from the Core thread, with the GUI thread ready to process events.
    void process(); ///< Processes toxcore events and ensure we stay connected, called by its own timer
    void bootstrapDht(); ///< Connects us to the Tox network

    QByteArray getToxSaveData(); ///< Returns the unencrypted tox save data

    void acceptFriendRequest(const QString& userId);
    void requestFriendship(const QString& friendAddress, const QString& message);
    void groupInviteFriend(uint32_t friendId, int groupId);
    int createGroup(uint8_t type = TOX_GROUPCHAT_TYPE_AV);

    void removeFriend(uint32_t friendId, bool fake = false);
    void removeGroup(int groupId, bool fake = false);

    void setStatus(Status status);
    void setUsername(const QString& username);
    void setStatusMessage(const QString& message);
    void setAvatar(const QByteArray& data);

     int sendMessage(uint32_t friendId, const QString& message);
    void sendGroupMessage(int groupId, const QString& message);
    void sendGroupAction(int groupId, const QString& message);
    void changeGroupTitle(int groupId, const QString& title);
     int sendAction(uint32_t friendId, const QString& action);
    void sendTyping(uint32_t friendId, bool typing);

    void sendFile(uint32_t friendId, QString Filename, QString FilePath, long long filesize);
    void sendAvatarFile(uint32_t friendId, const QByteArray& data);
    void cancelFileSend(uint32_t friendId, uint32_t fileNum);
    void cancelFileRecv(uint32_t friendId, uint32_t fileNum);
    void rejectFileRecvRequest(uint32_t friendId, uint32_t fileNum);
    void acceptFileRecvRequest(uint32_t friendId, uint32_t fileNum, QString path);
    void pauseResumeFileSend(uint32_t friendId, uint32_t fileNum);
    void pauseResumeFileRecv(uint32_t friendId, uint32_t fileNum);

    void answerCall(int callId);
    void rejectCall(int callId);
    void hangupCall(int callId);
    void startCall(uint32_t friendId, bool video=false);
    void cancelCall(int callId, uint32_t friendId);

    void micMuteToggle(int callId);
    void volMuteToggle(int callId);

    void setNospam(uint32_t nospam);

    bool isGroupAvEnabled(int groupId); ///< True for AV groups, false for text-only groups

    static void joinGroupCall(int groupId); ///< Starts a call in an existing AV groupchat. Call from the GUI thread.
    static void leaveGroupCall(int groupId); ///< Will not leave the group, just stop the call. Call from the GUI thread.
    static void disableGroupCallMic(int groupId);
    static void disableGroupCallVol(int groupId);
    static void enableGroupCallMic(int groupId);
    static void enableGroupCallVol(int groupId);
    static bool isGroupCallMicEnabled(int groupId);
    static bool isGroupCallVolEnabled(int groupId);

signals:
    void connected();
    void disconnected();

    void friendRequestReceived(const QString& userId, const QString& message);
    void friendMessageReceived(uint32_t friendId, const QString& message, bool isAction);

    void friendAdded(uint32_t friendId, const QString& userId);
    void friendshipChanged(uint32_t friendId);

    void friendStatusChanged(uint32_t friendId, Status status);
    void friendStatusMessageChanged(uint32_t friendId, const QString& message);
    void friendUsernameChanged(uint32_t friendId, const QString& username);
    void friendTypingChanged(uint32_t friendId, bool isTyping);
    void friendAvatarChanged(uint32_t friendId, const QPixmap& pic);
    void friendAvatarRemoved(uint32_t friendId);

    void friendRemoved(uint32_t friendId);

    void friendLastSeenChanged(uint32_t friendId, const QDateTime& dateTime);

    void emptyGroupCreated(int groupnumber);
    void groupInviteReceived(uint32_t friendId, uint8_t type, QByteArray publicKey);
    void groupMessageReceived(int groupnumber, int peernumber, const QString& message, bool isAction);
    void groupNamelistChanged(int groupnumber, int peernumber, uint8_t change);
    void groupTitleChanged(int groupnumber, const QString& author, const QString& title);
    void groupPeerAudioPlaying(int groupnumber, int peernumber);

    void usernameSet(const QString& username);
    void statusMessageSet(const QString& message);
    void statusSet(Status status);
    void idSet(const QString& id);
    void selfAvatarChanged(const QPixmap& pic);

    void messageSentResult(uint32_t friendId, const QString& message, int messageId);
    void groupSentResult(int groupId, const QString& message, int result);
    void actionSentResult(uint32_t friendId, const QString& action, int success);

    void receiptRecieved(int friedId, int receipt);

    void failedToAddFriend(const QString& userId, const QString& errorInfo = QString());
    void failedToRemoveFriend(uint32_t friendId);
    void failedToSetUsername(const QString& username);
    void failedToSetStatusMessage(const QString& message);
    void failedToSetStatus(Status status);
    void failedToSetTyping(bool typing);

    void failedToStart();
    void badProxy();

    void fileSendStarted(ToxFile file);
    void fileReceiveRequested(ToxFile file);
    void fileTransferAccepted(ToxFile file);
    void fileTransferCancelled(ToxFile file);
    void fileTransferFinished(ToxFile file);
    void fileUploadFinished(const QString& path);
    void fileDownloadFinished(const QString& path);
    void fileTransferPaused(ToxFile file);
    void fileTransferInfo(ToxFile file);
    void fileTransferRemotePausedUnpaused(ToxFile file, bool paused);
    void fileTransferBrokenUnbroken(ToxFile file, bool broken);

    void fileSendFailed(uint32_t friendId, const QString& fname);

    void avInvite(uint32_t friendId, int callIndex, bool video);
    void avStart(uint32_t friendId, int callIndex, bool video);
    void avCancel(uint32_t friendId, int callIndex);
    void avEnd(uint32_t friendId, int callIndex);
    void avRinging(uint32_t friendId, int callIndex, bool video);
    void avStarting(uint32_t friendId, int callIndex, bool video);
    void avEnding(uint32_t friendId, int callIndex);
    void avRequestTimeout(uint32_t friendId, int callIndex);
    void avPeerTimeout(uint32_t friendId, int callIndex);
    void avMediaChange(uint32_t friendId, int callIndex, bool videoEnabled);
    void avCallFailed(uint32_t friendId);
    void avRejected(uint32_t friendId, int callIndex);

    void videoFrameReceived(vpx_image* frame);

private:
    static void onFriendRequest(Tox* tox, const uint8_t* cUserId, const uint8_t* cMessage,
                                size_t cMessageSize, void* core);
    static void onFriendMessage(Tox* tox, uint32_t friendId, TOX_MESSAGE_TYPE type,
                                const uint8_t* cMessage, size_t cMessageSize, void* core);
    static void onFriendNameChange(Tox* tox, uint32_t friendId, const uint8_t* cName,
                                   size_t cNameSize, void* core);
    static void onFriendTypingChange(Tox* tox, uint32_t friendId, bool isTyping, void* core);
    static void onStatusMessageChanged(Tox* tox, uint32_t friendId, const uint8_t* cMessage,
                                       size_t cMessageSize, void* core);
    static void onUserStatusChanged(Tox* tox, uint32_t friendId, TOX_USER_STATUS userstatus, void* core);
    static void onConnectionStatusChanged(Tox* tox, uint32_t friendId, TOX_CONNECTION status, void* core);
    static void onGroupAction(Tox* tox, int groupnumber, int peernumber, const uint8_t * action,
                              uint16_t length, void* core);
    static void onGroupInvite(Tox *tox, int32_t friendId, uint8_t type, const uint8_t *data,
                              uint16_t length, void *userdata);
    static void onGroupMessage(Tox *tox, int groupnumber, int friendgroupnumber,
                               const uint8_t * message, uint16_t length, void *userdata);
    static void onGroupNamelistChange(Tox *tox, int groupId, int peerId, uint8_t change, void *core);
    static void onGroupTitleChange(Tox*, int groupnumber, int peernumber,
                                   const uint8_t* title, uint8_t len, void* _core);
    static void onReadReceiptCallback(Tox *tox, uint32_t friendId, uint32_t receipt, void *core);

    static void onAvInvite(void* toxav, int32_t call_index, void* core);
    static void onAvStart(void* toxav, int32_t call_index, void* core);
    static void onAvCancel(void* toxav, int32_t call_index, void* core);
    static void onAvReject(void* toxav, int32_t call_index, void* core);
    static void onAvEnd(void* toxav, int32_t call_index, void* core);
    static void onAvRinging(void* toxav, int32_t call_index, void* core);
    static void onAvRequestTimeout(void* toxav, int32_t call_index, void* core);
    static void onAvPeerTimeout(void* toxav, int32_t call_index, void* core);
    static void onAvMediaChange(void *toxav, int32_t call_index, void* core);

    static void sendGroupCallAudio(int groupId, ToxAv* toxav);

    static void prepareCall(uint32_t friendId, int callId, ToxAv *toxav, bool videoEnabled);
    static void cleanupCall(int callId);
    static void playCallAudio(void *toxav, int32_t callId, const int16_t *data,
                              uint16_t samples, void *user_data); // Callback
    static void sendCallAudio(int callId, ToxAv* toxav);
    static void playAudioBuffer(ALuint alSource, const int16_t *data, int samples,
                                unsigned channels, int sampleRate);
    static void playCallVideo(void *toxav, int32_t callId, const vpx_image_t* img, void *user_data);
    static void sendCallVideo(int callId, ToxAv* toxav, std::shared_ptr<VideoFrame> frame);

    bool checkConnection();

    void checkEncryptedHistory();
    void makeTox(QByteArray savedata);
    void loadFriends();

    void checkLastOnline(uint32_t friendId);

    void deadifyTox();

private slots:
    void killTimers(bool onlyStop); ///< Must only be called from the Core thread

private:
    Tox* tox;
    ToxAv* toxav;
    QTimer *toxTimer;
    Profile& profile;
    static ToxCall calls[TOXAV_MAX_CALLS];
#ifdef QTOX_FILTER_AUDIO
    static AudioFilterer * filterer[TOXAV_MAX_CALLS];
#endif
    static QHash<int, ToxGroupCall> groupCalls; // Maps group IDs to ToxGroupCalls
    QMutex messageSendMutex;
    bool ready;

    static const int videobufsize;
    static uint8_t* videobuf;

    static QThread *coreThread;

    friend class Audio; ///< Audio can access our calls directly to reduce latency
    friend class CoreFile; ///< CoreFile can access tox* and emit our signals
};

#endif // CORE_HPP

