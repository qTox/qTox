/*
    Copyright © 2014-2016 by The qTox Project

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

#include "friend.h"

#include <limits>

#include <tox/tox.h>

#include "src/core/core.h"
#include "src/core/corefile.h"
#include "src/core/cstring.h"
#include "src/core/cdata.h"
#include "src/group.h"
#include "src/persistence/settings.h"
#include "src/persistence/profile.h"
#include "src/nexus.h"

/**
 * @class Friend
 * @brief A self-managed friend database.
 *
 * All registered friends are kept in memory during the Tox profile's life-time.
 * Friends are not removed from the database unless @a Friend::destroy() is
 * Called.
 *
 * @fn Friend::toOneline(QString&)
 * @brief Replaces '\\n' with ' ' (whitespace) and removes '\\r'.
 * @note Modifies the original string.
 *
 * @fn Friend::get(const ToxId&)
 * @brief Looks up a friend in the friend list.
 * @param userId The lookup Tox Id.
 * @return the friend if found; nullptr otherwise.
 *
 * @fn Friend::remove(ID)
 * @brief Removes a friend from the list.
 * @note This will not unregister a friend from Tox.
 *
 * @fn Friend::remove(const ToxId&)
 * @brief Removes a friend from the list.
 * @note This will not unregister a friend from Tox.
 */

Friend::FriendCache Friend::friendList;
QHash<QString, Friend::ID> Friend::tox2id;
FriendNotify Friend::notifier;

/**
 * @brief Private implementation of the friend class
 */
class Friend::Private final : public QSharedData
{
    friend class QExplicitlySharedDataPointer<Friend::Private>;

public:
    Private(Friend::ID friendId, const ToxId& userId)
        : userId(userId)
        , friendId(friendId)
        , userName(Core::getInstance()->getPeerName(userId))
        , userAvatar(Nexus::getProfile()->loadAvatar(userId.publicKey))
        , friendStatus(Status::Offline)
        , isTyping(false)
        , offlineEngine(friendId)
    {
        if (userName.isEmpty())
            userName = userId.publicKey;

        friendList[friendId] = this;
        tox2id[userId.publicKey] = friendId;
    }

private:
    /**
     * @brief shared data destructor
     * @note The destructor is called by the shared pointer and must not be
     *       called manually.
     */
    ~Private()
    {
        tox2id.remove(userId.publicKey);
        friendList.remove(friendId);
    }

public:
    /**
     * @brief Invalidates this friend's ID and ToxID.
     * @note This is a shared instance and the pointer must not be free'd.
     *
     * Use this to remove a friend from the cache.
     */
    void invalidate()
    {
        tox2id.remove(userId.publicKey);
        friendList.remove(friendId);
        userId = ToxId();
        friendId = std::numeric_limits<Friend::ID>::max();
    }

    /**
     * @brief Change the real username of friend.
     * @param name New name to friend.
     */
    void setName(QString name)
    {
        if (name.isEmpty())
            name = userId.publicKey;

        if (userName != name)
        {
            userName = name;
            emit notifier.nameChanged(this, userName);
        }
    }

    /**
     * @brief Updates the friend's descriptive text.
     * @param message New status message.
     *
     * The status message is a brief descriptive text, describing the friend's
     * mood. Optional, but fun.
     */
    void setStatusMessage(const QString& message)
    {
        if (statusMessage != message)
        {
            statusMessage = message;
            emit notifier.statusMessageChanged(this, statusMessage);
        }
    }

    /**
     * @brief Updates the friend status.
     * @param s New friend status.
     */
    void setStatus(Status s)
    {
        if (friendStatus != s)
        {
            friendStatus = s;
            emit notifier.statusChanged(this, friendStatus);

            bool isConnectionStatus = s == Status::Online || s == Status::Offline;
            if (isConnectionStatus)
            {
                if (friendStatus == Status::Offline)
                {
                    QDateTime lastOnline = toxLastOnline();
                    if (lastOnline.isValid())
                        emit notifier.lastOnlineChanged(this, lastOnline);
                }

                CoreFile::onConnectionStatusChanged(friendId,
                                                    friendStatus != Status::Offline);
            }
        }
    }

    /**
     * @brief Updates the friend's typing status.
     * @param isTyping  the typing status
     */
    void setTyping(bool isTyping)
    {
        if (this->isTyping != isTyping)
        {
            this->isTyping = isTyping;
            emit notifier.typingChanged(this, isTyping);
        }
    }

    /**
     * @brief Returns the "last seen" time of this friend.
     * @return
     */
    QDateTime toxLastOnline() const
    {
        Tox* tox = Core::getInstance()->tox;
        uint64_t ts = tox_friend_get_last_online(tox, friendId, nullptr);
        return ts != std::numeric_limits<uint64_t>::max()
                     ? QDateTime::fromTime_t(static_cast<uint>(ts))
                     : QDateTime();
    }

    /**
      * @brief Cached lookup of a Tox friend.
      * @param friendId  the friend ID to look up
      * @return a registered friend object or a nullptr
      */
    static Private* lookup(Friend::ID friendId)
    {
        Private* p = friendList.value(friendId);
        if (p)
            return p;

        Tox* tox = Core::getInstance()->tox;
        assert(tox);

        uint8_t clientId[TOX_PUBLIC_KEY_SIZE];
        if (!tox_friend_get_public_key(tox, friendId, clientId, nullptr))
            return nullptr;

        p = new Private(friendId, ToxId(CUserId::toString(clientId)));

        const size_t nameSize = tox_friend_get_name_size(tox, friendId, nullptr);
        if (nameSize)
        {
            uint8_t* name = new uint8_t[nameSize];
            if (tox_friend_get_name(tox, friendId, name, nullptr))
            {
                p->userName =
                        CString::toString(name, static_cast<uint16_t>(nameSize));
            }

            delete[] name;
        }

        const size_t statusMessageSize =
                tox_friend_get_status_message_size(tox, friendId, nullptr);
        if (statusMessageSize)
        {
            uint8_t *statusMessage = new uint8_t[statusMessageSize];
            bool ok = tox_friend_get_status_message(tox, friendId,
                                                    statusMessage,
                                                    nullptr);
            if (ok)
            {
                p->statusMessage = CString::toString(
                                       statusMessage,
                                       static_cast<uint16_t>(statusMessageSize));
            }
            delete[] statusMessage;
        }

        return p;
    }

    /**
     * @brief Tox event callback
     * @sa tox_friend_status_message
     */
    static void cbName(Tox* tox, Friend::ID friendId, const uint8_t* name,
                       size_t len, void* payload)
    {
        Q_UNUSED(tox);
        Q_UNUSED(payload);

        Private* p = friendList.value(friendId);
        if (p)
        {
            // friend is instantiated -> sync & emit signal
            p->setName(CString::toString(name, static_cast<uint16_t>(len)));
        }
    }

    /**
     * @brief Tox event callback
     * @sa tox_friend_status_message
     */
    static void cbStatusMessage(Tox* tox, Friend::ID friendId,
                                const uint8_t* message,
                                size_t len, void* payload)
    {
        Q_UNUSED(payload);
        if (!Core::getInstance()->checkTox(tox))
            return;

        Private* p = friendList.value(friendId);
        if (p)
        {
            // friend is instantiated -> sync & emit signal
            p->setStatusMessage(
                        CString::toString(message, static_cast<uint16_t>(len)));
        }
    }

    /**
     * @brief Tox event callback
     * @sa tox_friend_status
     */
    static void cbStatus(Tox* tox, Friend::ID friendId,
                         TOX_USER_STATUS status, void* payload)
    {
        Q_UNUSED(payload);
        if (!Core::getInstance()->checkTox(tox))
            return;

        Private* p = friendList.value(friendId);
        if (p)
        {
            // friend is instantiated -> sync & emit signal
            switch (status)
            {
            case TOX_USER_STATUS_NONE:
                p->setStatus(Status::Online);
                break;
            case TOX_USER_STATUS_AWAY:
                p->setStatus(Status::Away);
                break;
            case TOX_USER_STATUS_BUSY:
                p->setStatus(Status::Busy);
                break;
            }
        }
    }

    /**
     * @brief Tox event callback
     * @sa tox_friend_typing
     */
    static void cbTyping(Tox* tox, Friend::ID friendId, bool isTyping,
                         void* payload)
    {
        Q_UNUSED(payload);
        if (!Core::getInstance()->checkTox(tox))
            return;

        Private* p = lookup(friendId);
        if (p)
        {
            // friend is instantiated -> sync & emit signal
            p->setTyping(isTyping);
        }
    }

    /**
     * @brief Tox event callback
     * @sa tox_friend_connection_status
     */
    static void cbConnectionStatus(Tox* tox, Friend::ID friendId,
                                   TOX_CONNECTION status,
                                   void* payload)
    {
        Q_UNUSED(payload);
        if (!Core::getInstance()->checkTox(tox))
            return;

        Private* p = friendList.value(friendId);
        if (p)
        {
            // friend is instantiated -> sync & emit signal
            Status friendStatus = status != TOX_CONNECTION_NONE ? Status::Online
                                                                : Status::Offline;

            p->setStatus(friendStatus);
        }
    }

public:
    ToxId userId;
    Friend::ID friendId;
    QString userName;
    QString statusMessage;
    QPixmap userAvatar;
    Status friendStatus;
    bool isTyping;
    OfflineMsgEngine offlineEngine;
};

Friend::Friend(const Friend& other)
    : data(other.data)
{
}

Friend::Friend(Friend &&other)
{
    data = std::move(other.data);
}

/**
 * @brief destructor
 */
Friend::~Friend()
{
}

Friend& Friend::operator=(const Friend& other)
{
    data = other.data;
    return *this;
}

Friend& Friend::operator=(Friend&& other)
{
    data = std::move(other.data);
    return *this;
}

/**
 * @brief constructor
 * @param p the private pointer.
 */
Friend::Friend(Friend::Private *p)
    : data(p)
{
}

/**
 * @brief Looks up a friend in the friend list.
 * @param friendId The lookup ID.
 * @return The friend if found; nullptr otherwise.
 */
Friend Friend::get(Friend::ID friendId)
{
    return Private::lookup(friendId);
}

/**
 * @brief Removes a friend from the local Tox profile.
 * @param friendId  the ID of the friend to remove
 *
 * The friend is removed from the local profile and also from the cache.
 */
void Friend::deleteFromProfile(Friend::ID friendId)
{
    Core* core = Core::getInstance();

    if (tox_friend_delete(core->tox, friendId, nullptr))
    {
        core->profile.saveToxSave();
        Private* p = friendList.value(friendId);
        if (p)
            p->invalidate();

        emit notifier.removed(friendId);
    }
    else
    {
        emit notifier.failedToRemove(friendId);
    }
}

/**
 * @brief Updates the friend's cached avatar.
 * @param avatar the avatar
 */
void Friend::updateAvatar(Friend::ID friendId, const QPixmap& avatar)
{
    Private* p = friendList.value(friendId);
    if (p && (avatar.cacheKey() != p->userAvatar.cacheKey()))
    {
        p->userAvatar = avatar;
        emit notifier.avatarChanged(p, p->userAvatar);
    }
}

Friend::IDList Friend::idList()
{
    Tox* tox = Core::getInstance()->tox;
    IDList ids(static_cast<int>(tox_self_get_friend_list_size(tox)));
    tox_self_get_friend_list(tox, ids.data());
    return ids;
}

void Friend::initCallbacks()
{
    Tox* tox = Core::getInstance()->tox;
    assert(tox);

    tox_callback_friend_name(tox, Private::cbName, nullptr);
    tox_callback_friend_status_message(tox, Private::cbStatusMessage, nullptr);
    tox_callback_friend_status(tox, Private::cbStatus, nullptr);
    tox_callback_friend_connection_status(tox, Private::cbConnectionStatus,
                                          nullptr);
    tox_callback_friend_typing(tox, Private::cbTyping, nullptr);
}

void Friend::initCache()
{
    // TODO: REMOVE THIS FUNCTION COMPLETELY!
    // This is a dirty hack to initialize the friend cache with every
    // available friend. Instead, just the ID's should be loaded and
    // friends are cached "automagically" via Friend::Private::lookup

    // clear a possibly previous build cache.
    while (friendList.size() > 0)
    {
        auto it = friendList.begin();
        (*it)->invalidate();
    }

    for (ID friendId : idList())
    {
        // important note:
        // this pointer is deleted when it goes out of scope (p->ref == 0)
        Private* p = Private::lookup(friendId);
        if (p)
            emit notifier.added(p);
    }
}

/**
 * @brief Loads the friend's chat history if enabled.
 */
void Friend::loadHistory()
{
    // TODO: Actually load the friend's history instead of just emitting
    if (Nexus::getProfile()->isHistoryEnabled())
        emit notifier.loadHistory(*this);
}

/**
 * @brief invalidates the ToxID and removes a friend from the friend cache.
 * @note The friend is only removed from the cache and still available not from
 *       in the local Tox profile.
 * @sa deleteFromTox
 */
void Friend::invalidate()
{
    if (data)
        data->invalidate();

    data.reset();
}

/**
 * @brief Set an alias (displayed name) for friend.
 * @param alias New alias to friend.
 * @note The alias length is restricted to @a TOX_MAX_NAME_LENGTH characters.
 *
 * The alias can is stored in local settings and is not transferred over the
 * Tox network.
 */
void Friend::setAlias(QString alias)
{
    if (data)
    {
        // TODO: handle alias completely in Settings
        Settings& s = Settings::getInstance();
        QString oldAlias = s.getFriendAlias(data->userId);

        alias = alias.left(TOX_MAX_NAME_LENGTH);
        if (oldAlias != alias)
        {
            s.setFriendAlias(data->userId, alias);
            s.savePersonal();
            emit notifier.aliasChanged(*this, alias);
        }
    }
}

/**
 * @brief Get status message.
 * @return Friend status message.
 */
QString Friend::getStatusMessage()
{
    if (!data)
        return QString();

    return data->statusMessage;
}

/**
 * @brief Returns name, which should be displayed.
 * @return Friend displayed name.
 *
 * Return friend alias if setted, username otherwise.
 */
QString Friend::getDisplayedName() const
{
    if (!data)
        return QString();

    QString alias = Settings::getInstance().getFriendAlias(data->userId);
    return alias.isEmpty() ? data->userName : alias;
}

/**
 * @brief Checks, if friend has alias.
 * @return True, if user sets alias for this friend, false otherwise.
 */
bool Friend::hasAlias() const
{
    if (data)
        return !Settings::getInstance().getFriendAlias(data->userId).isEmpty();

    return false;
}

/**
 * @brief The friend's avatar.
 * @return the avatar or a null pixmap
 */
QPixmap Friend::getAvatar() const
{
    return data ? data->userAvatar : QPixmap();
}

/**
 * @brief Get ToxId
 * @return ToxId of current friend.
 */
ToxId Friend::getToxId() const
{
    return data ? data->userId : ToxId();
}

/**
 * @brief Get friend id.
 * @return Friend id.
 */
Friend::ID Friend::getFriendId() const
{
    if (!data)
        return std::numeric_limits<Friend::ID>::max();

    return data->friendId;
}

/**
 * @brief Get friend status.
 * @return Status of current friend.
 */
Status Friend::getStatus() const
{
    if (!data)
        return Status::Offline;

    return data->friendStatus;
}

/**
 * @brief Returns the frend's typing status.
 * @return the typing status.
 */
bool Friend::getTyping() const
{
    return data ? data->isTyping : false;
}

/**
 * @brief Returns the friend's @a OfflineMessageEngine.
 * @return A const reference to the offline engine
 */
const OfflineMsgEngine& Friend::getOfflineMsgEngine() const
{
    return data->offlineEngine;
}

/**
 * @brief Register new offline message in @a OfflineMsgEngine.
 * @param rec Receipt ID.
 * @param id Message ID.
 * @param msg Message to register.
 */
void Friend::registerReceipt(int rec, qint64 id, ChatMessage::Ptr msg)
{
    if (!data)
        return;

    data->offlineEngine.registerReceipt(rec, id, msg);
}

/**
 * @brief Discharge offline message from @a OfflineMsgEngine.
 * @param receipt Receipt ID.
 */
void Friend::dischargeReceipt(int receipt)
{
    if (!data)
        return;

    data->offlineEngine.dischargeReceipt(receipt);
}

/**
 * @brief Remove all offline receipts from @a OfflineMsgEngine.
 */
void Friend::clearOfflineReceipts()
{
    if (!data)
        return;

    data->offlineEngine.removeAllReceipts();
}

/**
 * @brief Deliver all offline messages from @a OfflineMsgEngine.
 */
void Friend::deliverOfflineMsgs()
{
    if (!data)
        return;

    data->offlineEngine.deliverOfflineMsgs();
}

/**
 * @brief constructor
 *
 * Registers the @a Friend class to Qt's meta object system.
 */
FriendNotify::FriendNotify()
{
    qRegisterMetaType<ToxId>("ToxId");
    qRegisterMetaType<Friend::ID>("Friend::ID");
    qRegisterMetaType<Friend>("Friend");
}
