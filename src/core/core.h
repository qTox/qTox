/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2017 by The qTox Project Contributors

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

#include "toxfile.h"
#include "toxid.h"

#include <tox/tox.h>

#include <QMutex>
#include <QObject>

#include <functional>

class CoreAV;
class ICoreSettings;
class GroupInvite;
class Profile;
class QTimer;

enum class Status
{
    Online = 0,
    Away,
    Busy,
    Offline
};

class Core : public QObject
{
    Q_OBJECT
public:
    Core(QThread* coreThread, Profile& profile, const ICoreSettings* const settings);
    static Core* getInstance();
    const CoreAV* getAv() const;
    CoreAV* getAv();
    ~Core();

    static const QString TOX_EXT;
    static QStringList splitMessage(const QString& message, int maxLen);

    QString getPeerName(const ToxPk& id) const;

    QVector<uint32_t> getFriendList() const;
    uint32_t getGroupNumberPeers(int groupId) const;
    QString getGroupPeerName(int groupId, int peerId) const;
    ToxPk getGroupPeerPk(int groupId, int peerId) const;
    QStringList getGroupPeerNames(int groupId) const;
    ToxPk getFriendPublicKey(uint32_t friendNumber) const;
    QString getFriendUsername(uint32_t friendNumber) const;

    bool isFriendOnline(uint32_t friendId) const;
    bool hasFriendWithPublicKey(const ToxPk& publicKey) const;
    uint32_t joinGroupchat(const GroupInvite& inviteInfo) const;
    void quitGroupChat(int groupId) const;

    QString getUsername() const;
    Status getStatus() const;
    QString getStatusMessage() const;
    ToxId getSelfId() const;
    ToxPk getSelfPublicKey() const;
    QPair<QByteArray, QByteArray> getKeypair() const;

    bool isReady() const;

    void sendFile(uint32_t friendId, QString filename, QString filePath, long long filesize);

public slots:
    void start(const QByteArray& savedata);
    void reset();
    void process();
    void bootstrapDht();

    QByteArray getToxSaveData();

    void acceptFriendRequest(const ToxPk& friendPk);
    void requestFriendship(const ToxId& friendAddress, const QString& message);
    void groupInviteFriend(uint32_t friendId, int groupId);
    int createGroup(uint8_t type = TOX_CONFERENCE_TYPE_AV);

    void removeFriend(uint32_t friendId, bool fake = false);
    void removeGroup(int groupId, bool fake = false);

    void setStatus(Status status);
    void setUsername(const QString& username);
    void setStatusMessage(const QString& message);

    int sendMessage(uint32_t friendId, const QString& message);
    void sendGroupMessage(int groupId, const QString& message);
    void sendGroupAction(int groupId, const QString& message);
    void changeGroupTitle(int groupId, const QString& title);
    int sendAction(uint32_t friendId, const QString& action);
    void sendTyping(uint32_t friendId, bool typing);

    void sendAvatarFile(uint32_t friendId, const QByteArray& data);
    void cancelFileSend(uint32_t friendId, uint32_t fileNum);
    void cancelFileRecv(uint32_t friendId, uint32_t fileNum);
    void rejectFileRecvRequest(uint32_t friendId, uint32_t fileNum);
    void acceptFileRecvRequest(uint32_t friendId, uint32_t fileNum, QString path);
    void pauseResumeFileSend(uint32_t friendId, uint32_t fileNum);
    void pauseResumeFileRecv(uint32_t friendId, uint32_t fileNum);

    void setNospam(uint32_t nospam);

signals:
    void connected();
    void disconnected();

    void friendRequestReceived(const ToxPk& friendPk, const QString& message);
    void friendMessageReceived(uint32_t friendId, const QString& message, bool isAction);

    void friendAdded(uint32_t friendId, const ToxPk& friendPk);
    void requestSent(const ToxPk& friendPk, const QString& message);

    void friendStatusChanged(uint32_t friendId, Status status);
    void friendStatusMessageChanged(uint32_t friendId, const QString& message);
    void friendUsernameChanged(uint32_t friendId, const QString& username);
    void friendTypingChanged(uint32_t friendId, bool isTyping);
    void friendAvatarChanged(uint32_t friendId, const QPixmap& pic);
    void friendAvatarRemoved(uint32_t friendId);

    void friendRemoved(uint32_t friendId);

    void friendLastSeenChanged(uint32_t friendId, const QDateTime& dateTime);

    void emptyGroupCreated(int groupnumber);
    void groupInviteReceived(const GroupInvite& inviteInfo);
    void groupMessageReceived(int groupnumber, int peernumber, const QString& message, bool isAction);
    void groupNamelistChanged(int groupnumber, int peernumber, uint8_t change);
    void groupTitleChanged(int groupnumber, const QString& author, const QString& title);
    void groupPeerAudioPlaying(int groupnumber, int peernumber);

    void usernameSet(const QString& username);
    void statusMessageSet(const QString& message);
    void statusSet(Status status);
    void idSet(const ToxId& id);

    void messageSentResult(uint32_t friendId, const QString& message, int messageId);
    void groupSentFailed(int groupId);
    void actionSentResult(uint32_t friendId, const QString& action, int success);

    void receiptRecieved(int friedId, int receipt);

    void failedToAddFriend(const ToxPk& friendPk, const QString& errorInfo = QString());
    void failedToRemoveFriend(uint32_t friendId);
    void failedToSetUsername(const QString& username);
    void failedToSetStatusMessage(const QString& message);
    void failedToSetStatus(Status status);
    void failedToSetTyping(bool typing);

    void failedToStart();
    void badProxy();
    void avReady();

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
    static void onUserStatusChanged(Tox* tox, uint32_t friendId, TOX_USER_STATUS userstatus,
                                    void* core);
    static void onConnectionStatusChanged(Tox* tox, uint32_t friendId, TOX_CONNECTION status,
                                          void* core);
    static void onGroupInvite(Tox* tox, uint32_t friendId, TOX_CONFERENCE_TYPE type,
                              const uint8_t* cookie, size_t length, void* vCore);
    static void onGroupMessage(Tox* tox, uint32_t groupId, uint32_t peerId, TOX_MESSAGE_TYPE type,
                               const uint8_t* cMessage, size_t length, void* vCore);
    static void onGroupNamelistChange(Tox* tox, uint32_t groupId, uint32_t peerId,
                                      TOX_CONFERENCE_STATE_CHANGE change, void* core);
    static void onGroupTitleChange(Tox* tox, uint32_t groupId, uint32_t peerId,
                                   const uint8_t* cTitle, size_t length, void* vCore);
    static void onReadReceiptCallback(Tox* tox, uint32_t friendId, uint32_t receipt, void* core);

    void sendGroupMessageWithType(int groupId, const QString& message, TOX_MESSAGE_TYPE type);
    bool parsePeerQueryError(TOX_ERR_CONFERENCE_PEER_QUERY error) const;
    bool parseConferenceJoinError(TOX_ERR_CONFERENCE_JOIN error) const;
    bool checkConnection();

    void checkEncryptedHistory();
    void makeTox(QByteArray savedata);
    void makeAv();
    void loadFriends();

    void checkLastOnline(uint32_t friendId);

    void deadifyTox();
    QString getFriendRequestErrorMessage(const ToxId& friendId, const QString& message) const;

private slots:
    void killTimers(bool onlyStop);

private:
    Tox* tox;
    CoreAV* av;
    QTimer* toxTimer;
    Profile& profile;
    QMutex messageSendMutex;
    bool ready;
    const ICoreSettings* const s;

    static QThread* coreThread;

    friend class Audio;    ///< Audio can access our calls directly to reduce latency
    friend class CoreFile; ///< CoreFile can access tox* and emit our signals
    friend class CoreAV;   ///< CoreAV accesses our toxav* for now
};

#endif // CORE_HPP
