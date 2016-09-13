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

#include "src/core/core.h"
#include "src/group.h"
#include "src/persistence/settings.h"
#include "src/persistence/profile.h"
#include "src/nexus.h"

class Friend::Private
{
public:
    Private(Friend::ID friendId, const ToxId& userId)
        : userName(Core::getInstance()->getPeerName(userId))
        , userAlias(Settings::getInstance().getFriendAlias(userId))
        , userId(userId)
        , friendId(friendId)
        , hasNewEvents(0)
        , friendStatus(Status::Offline)
        , offlineEngine(new Friend(this))
    {
        if (userName.isEmpty())
            userName = userId.publicKey;
    }

    QString userName;
    QString userAlias;
    QString statusMessage;
    ToxId userId;
    Friend::ID friendId;
    bool hasNewEvents;
    Status friendStatus;
    OfflineMsgEngine offlineEngine;
};

QHash<Friend::ID, Friend::Private*> Friend::friendList;
QHash<QString, Friend::ID> Friend::tox2id;

/**
 * @brief Friend constructor.
 * @param friendId The friend's ID.
 * @param userId The friend's ToxId.
 *
 * Add new friend in the friend list.
 */
Friend::Friend(Friend::ID friendId, const ToxId& userId)
{
    auto friendChecker = friendList.find(friendId);
    if (friendChecker != friendList.end())
        qWarning() << "addFriend: friendId already taken";

    data = new Friend::Private(friendId, userId);
    friendList[friendId] = data;
    tox2id[userId.publicKey] = friendId;
}

/**
 * @brief Friend constructor, without adding friend to db.
 * @param data Private friend data.
 */
Friend::Friend(Friend::Private *data)
    : data(data)
{
}

/**
 * @brief Friend destructor.
 *
 * Removes a friend from the friend list.
 */
Friend::~Friend()
{
    auto f_it = friendList.find(data->friendId);
    if (f_it == friendList.end())
        return;

    delete *f_it;
    friendList.erase(f_it);
}

/**
 * @brief Looks up a friend in the friend list.
 * @param friendId The lookup ID.
 * @return The friend if found; nullptr otherwise.
 */
Friend* Friend::get(Friend::ID friendId)
{
    auto f_it = friendList.find(friendId);
    if (f_it == friendList.end())
        return nullptr;

    Friend* f = new Friend(*f_it);
    return f;
}

/**
 * @brief Looks up a friend in the friend list.
 * @param userId The lookup Tox Id.
 * @return the friend if found; nullptr otherwise.
 */
Friend* Friend::get(const ToxId& userId)
{
    auto id = tox2id.find(userId.publicKey);
    if (id == tox2id.end())
        return nullptr;

    Friend *f = get(*id);
    if (f && f->getToxId() == userId)
        return f;

    return nullptr;
}

/**
 * @brief Get list of all existing friends.
 * @return List of all existing friends.
 */
QList<Friend*> Friend::getAll()
{
    QList<Friend*> result;
    for (Friend::Private* data : friendList)
        result.append(new Friend(data));

    return result;
}

/**
 * @brief Loads the friend's chat history if enabled.
 */
void Friend::loadHistory()
{
    if (Nexus::getProfile()->isHistoryEnabled())
        emit loadChatHistory();
}

/**
 * @brief Change the real username of friend.
 * @param name New name to friend.
 */
void Friend::setName(QString name)
{
    if (name.isEmpty())
        name = data->userId.publicKey;

    if (data->userName != name)
    {
        data->userName = name;
        emit nameChanged(name);
    }
}

/**
 * @brief Set new displayed name to friend.
 * @param alias New alias to friend.
 *
 * Alias will override friend name in friend list.
 */
void Friend::setAlias(QString alias)
{
    if (data->userAlias != alias)
    {
        data->userAlias = alias;
        emit aliasChanged(data->friendId, alias);
    }
}

/**
 * @brief Sets a descriptive status message.
 * @param message New status message.
 *
 * The status message is a brief descriptive text, describing the friend's mood.
 * Optional, but fun.
 */
void Friend::setStatusMessage(QString message)
{
    if (data->statusMessage != message)
    {
        data->statusMessage = message;
        emit newStatusMessage(message);
    }
}

/**
 * @brief Get status message.
 * @return Friend status message.
 */
QString Friend::getStatusMessage()
{
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
    return data->userAlias.isEmpty() ? data->userName : data->userAlias;
}

/**
 * @brief Checks, if friend has alias.
 * @return True, if user sets alias for this friend, false otherwise.
 */
bool Friend::hasAlias() const
{
    return !data->userAlias.isEmpty();
}

/**
 * @brief Get ToxId
 * @return ToxId of current friend.
 */
const ToxId& Friend::getToxId() const
{
    return data->userId;
}

/**
 * @brief Get friend id.
 * @return Friend id.
 */
uint32_t Friend::getFriendId() const
{
    return data->friendId;
}

/**
 * @brief Set event flag.
 * @param flag True if friend has new event, false otherwise.
 */
void Friend::setEventFlag(bool flag)
{
    data->hasNewEvents = flag;
}

/**
 * @brief Get event flag.
 * @return Return true, if friend has new event, false otherwise.
 */
bool Friend::getEventFlag() const
{
    return data->hasNewEvents;
}

/**
 * @brief Set friend status.
 * @param s New friend status.
 */
void Friend::setStatus(Status s)
{
    if (data->friendStatus != s)
    {
        data->friendStatus = s;
        emit statusChanged(data->friendId, data->friendStatus);
    }
}

/**
 * @brief Get friend status.
 * @return Status of current friend.
 */
Status Friend::getStatus() const
{
    return data->friendStatus;
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
    data->offlineEngine.registerReceipt(rec, id, msg);
}

/**
 * @brief Discharge offline message from @a OfflineMsgEngine.
 * @param receipt Receipt ID.
 */
void Friend::dischargeReceipt(int receipt)
{
    data->offlineEngine.dischargeReceipt(receipt);
}

/**
 * @brief Remove all offline receipts from @a OfflineMsgEngine.
 */
void Friend::clearOfflineReceipts()
{
    data->offlineEngine.removeAllReceipts();
}

/**
 * @brief Deliver all offline messages from @a OfflineMsgEngine.
 */
void Friend::deliverOfflineMsgs()
{
    data->offlineEngine.deliverOfflineMsgs();
}
