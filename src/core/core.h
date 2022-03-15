/*
    Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
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

#pragma once

#include "groupid.h"
#include "icorefriendmessagesender.h"
#include "icoregroupmessagesender.h"
#include "icoregroupquery.h"
#include "icoreidhandler.h"
#include "receiptnum.h"
#include "toxfile.h"
#include "toxid.h"
#include "toxpk.h"

#include "util/strongtype.h"
#include "util/compatiblerecursivemutex.h"
#include "src/model/status.h"
#include <tox/tox.h>

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>

#include <functional>
#include <memory>

class CoreAV;
class CoreFile;
class CoreExt;
class IAudioControl;
class ICoreSettings;
class GroupInvite;
class Profile;
class Core;
class IBootstrapListGenerator;
struct DhtServer;

using ToxCorePtr = std::unique_ptr<Core>;

class Core : public QObject,
             public ICoreFriendMessageSender,
             public ICoreIdHandler,
             public ICoreGroupMessageSender,
             public ICoreGroupQuery
{
    Q_OBJECT
public:
    enum class ToxCoreErrors
    {
        BAD_PROXY,
        INVALID_SAVE,
        FAILED_TO_START,
        ERROR_ALLOC
    };

    static ToxCorePtr makeToxCore(const QByteArray& savedata, const ICoreSettings& settings,
                                  IBootstrapListGenerator& bootstrapNodes, ToxCoreErrors* err = nullptr);
    const CoreAV* getAv() const;
    CoreAV* getAv();
    void setAv(CoreAV* coreAv);

    CoreFile* getCoreFile() const;
    Tox* getTox() const;
    CompatibleRecursiveMutex& getCoreLoopLock() const;

    const CoreExt* getExt() const;
    CoreExt* getExt();
    ~Core();

    static const QString TOX_EXT;
    uint64_t getMaxMessageSize() const;
    QString getPeerName(const ToxPk& id) const;
    QVector<uint32_t> getFriendList() const;
    GroupId getGroupPersistentId(uint32_t groupNumber) const override;
    uint32_t getGroupNumberPeers(int groupId) const override;
    QString getGroupPeerName(int groupId, int peerId) const override;
    ToxPk getGroupPeerPk(int groupId, int peerId) const override;
    QStringList getGroupPeerNames(int groupId) const override;
    bool getGroupAvEnabled(int groupId) const override;
    ToxPk getFriendPublicKey(uint32_t friendNumber) const;
    QString getFriendUsername(uint32_t friendNumber) const;

    bool isFriendOnline(uint32_t friendId) const;
    bool hasFriendWithPublicKey(const ToxPk& publicKey) const;
    uint32_t joinGroupchat(const GroupInvite& inviteInfo);
    void quitGroupChat(int groupId) const;

    QString getUsername() const override;
    Status::Status getStatus() const;
    QString getStatusMessage() const;
    ToxId getSelfId() const override;
    ToxPk getSelfPublicKey() const override;
    QPair<QByteArray, QByteArray> getKeypair() const;

    QByteArray getSelfDhtId() const;
    int getSelfUdpPort() const;

public slots:
    void start();

    QByteArray getToxSaveData();

    void acceptFriendRequest(const ToxPk& friendPk);
    void requestFriendship(const ToxId& friendId, const QString& message);
    void groupInviteFriend(uint32_t friendId, int groupId);
    int createGroup(uint8_t type = TOX_CONFERENCE_TYPE_AV);

    void removeFriend(uint32_t friendId);
    void removeGroup(int groupId);

    void setStatus(Status::Status status);
    void setUsername(const QString& username);
    void setStatusMessage(const QString& message);

    bool sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt) override;
    void sendGroupMessage(int groupId, const QString& message) override;
    void sendGroupAction(int groupId, const QString& message) override;
    void changeGroupTitle(int groupId, const QString& title);
    bool sendAction(uint32_t friendId, const QString& action, ReceiptNum& receipt) override;
    void sendTyping(uint32_t friendId, bool typing);

    void setNospam(uint32_t nospam);

signals:
    void connected();
    void disconnected();

    void friendRequestReceived(const ToxPk& friendPk, const QString& message);
    void friendAvatarChanged(const ToxPk& friendPk, const QByteArray& pic);
    void friendAvatarRemoved(const ToxPk& friendPk);

    void requestSent(const ToxPk& friendPk, const QString& message);
    void failedToAddFriend(const ToxPk& friendPk, const QString& errorInfo = QString());

    void usernameSet(const QString& username);
    void statusMessageSet(const QString& message);
    void statusSet(Status::Status status);
    void idSet(const ToxId& id);

    void failedToSetUsername(const QString& username);
    void failedToSetStatusMessage(const QString& message);
    void failedToSetStatus(Status::Status status);
    void failedToSetTyping(bool typing);

    void saveRequest();

    /**
     * @deprecated prefer signals using ToxPk
     */

    void fileAvatarOfferReceived(uint32_t friendId, uint32_t fileId, const QByteArray& avatarHash, uint64_t filesize);

    void friendMessageReceived(uint32_t friendId, const QString& message, bool isAction);
    void friendAdded(uint32_t friendId, const ToxPk& friendPk);

    void friendStatusChanged(uint32_t friendId, Status::Status status);
    void friendStatusMessageChanged(uint32_t friendId, const QString& message);
    void friendUsernameChanged(uint32_t friendId, const QString& username);
    void friendTypingChanged(uint32_t friendId, bool isTyping);

    void friendRemoved(uint32_t friendId);
    void friendLastSeenChanged(uint32_t friendId, const QDateTime& dateTime);

    void emptyGroupCreated(int groupnumber, const GroupId groupId, const QString& title = QString());
    void groupInviteReceived(const GroupInvite& inviteInfo);
    void groupMessageReceived(int groupnumber, int peernumber, const QString& message, bool isAction);
    void groupNamelistChanged(int groupnumber, int peernumber, uint8_t change);
    void groupPeerlistChanged(int groupnumber);
    void groupPeerNameChanged(int groupnumber, const ToxPk& peerPk, const QString& newName);
    void groupTitleChanged(int groupnumber, const QString& author, const QString& title);
    void groupPeerAudioPlaying(int groupnumber, ToxPk peerPk);
    void groupSentFailed(int groupId);
    void groupJoined(int groupnumber, GroupId groupId);
    void actionSentResult(uint32_t friendId, const QString& action, int success);

    void receiptRecieved(int friedId, ReceiptNum receipt);

    void failedToRemoveFriend(uint32_t friendId);

private:
    Core(QThread* coreThread_, IBootstrapListGenerator& bootstrapListGenerator_, const ICoreSettings& settings_);

    static void onFriendRequest(Tox* tox, const uint8_t* cFriendPk, const uint8_t* cMessage,
                                size_t cMessageSize, void* core);
    static void onFriendMessage(Tox* tox, uint32_t friendId, Tox_Message_Type type,
                                const uint8_t* cMessage, size_t cMessageSize, void* core);
    static void onFriendNameChange(Tox* tox, uint32_t friendId, const uint8_t* cName,
                                   size_t cNameSize, void* core);
    static void onFriendTypingChange(Tox* tox, uint32_t friendId, bool isTyping, void* core);
    static void onStatusMessageChanged(Tox* tox, uint32_t friendId, const uint8_t* cMessage,
                                       size_t cMessageSize, void* core);
    static void onUserStatusChanged(Tox* tox, uint32_t friendId, Tox_User_Status userstatus,
                                    void* core);
    static void onConnectionStatusChanged(Tox* tox, uint32_t friendId, Tox_Connection status,
                                          void* vCore);
    static void onGroupInvite(Tox* tox, uint32_t friendId, Tox_Conference_Type type,
                              const uint8_t* cookie, size_t length, void* vCore);
    static void onGroupMessage(Tox* tox, uint32_t groupId, uint32_t peerId, Tox_Message_Type type,
                               const uint8_t* cMessage, size_t length, void* vCore);
    static void onGroupPeerListChange(Tox* tox, uint32_t groupId, void* core);
    static void onGroupPeerNameChange(Tox* tox, uint32_t groupId, uint32_t peerId, const uint8_t* name,
                                      size_t length, void* core);
    static void onGroupTitleChange(Tox* tox, uint32_t groupId, uint32_t peerId,
                                   const uint8_t* cTitle, size_t length, void* vCore);

    static void onLosslessPacket(Tox* tox, uint32_t friendId,
                                   const uint8_t* data, size_t length, void* core);
    static void onReadReceiptCallback(Tox* tox, uint32_t friendId, uint32_t receipt, void* core);

    void sendGroupMessageWithType(int groupId, const QString& message, Tox_Message_Type type);
    bool sendMessageWithType(uint32_t friendId, const QString& message, Tox_Message_Type type, ReceiptNum& receipt);
    bool checkConnection();

    void makeTox(QByteArray savedata, ICoreSettings* s);
    void loadFriends();
    void loadGroups();
    void bootstrapDht();

    void checkLastOnline(uint32_t friendId);

    QString getFriendRequestErrorMessage(const ToxId& friendId, const QString& message) const;
    static void registerCallbacks(Tox* tox);

private slots:
    void process();
    void onStarted();

private:
    struct ToxDeleter
    {
        void operator()(Tox* tox_)
        {
            tox_kill(tox_);
        }
    };
    /* Using the now commented out statements in checkConnection(), I watched how
    * many ticks disconnects-after-initial-connect lasted. Out of roughly 15 trials,
    * 5 disconnected; 4 were DCd for less than 20 ticks, while the 5th was ~50 ticks.
    * So I set the tolerance here at 25, and initial DCs should be very rare now.
    * This should be able to go to 50 or 100 without affecting legitimate disconnects'
    * downtime, but lets be conservative for now. Edit: now ~~40~~ 30.
    */
    #define CORE_DISCONNECT_TOLERANCE 30

    using ToxPtr = std::unique_ptr<Tox, ToxDeleter>;
    ToxPtr tox;

    std::unique_ptr<CoreFile> file;
    CoreAV* av = nullptr;
    std::unique_ptr<CoreExt> ext;
    QTimer* toxTimer = nullptr;
    // recursive, since we might call our own functions
    mutable CompatibleRecursiveMutex coreLoopLock;

    std::unique_ptr<QThread> coreThread;
    const IBootstrapListGenerator& bootstrapListGenerator;
    const ICoreSettings& settings;
    bool isConnected = false;
    int tolerance = CORE_DISCONNECT_TOLERANCE;
};
