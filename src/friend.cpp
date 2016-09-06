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
#include "friendlist.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "src/persistence/settings.h"
#include "src/persistence/profile.h"
#include "src/nexus.h"

/**
 * @brief Looks up a friend in the friend list.
 * @param friendId  the lookup ID
 * @return the friend if found; nullptr otherwise
 */
Friend* Friend::get(int friendId)
{
    return FriendList::findFriend(friendId);
}

/**
 * @brief Removes a friend from the friend list.
 * @param friendId  the friend's ID
 */
void Friend::remove(int friendId)
{
    FriendList::removeFriend(friendId);
}

Friend::Friend(uint32_t FriendId, const ToxId &UserId)
    : userName{Core::getInstance()->getPeerName(UserId)}
    , userAlias(Settings::getInstance().getFriendAlias(UserId))
    , userID(UserId)
    , friendId(FriendId)
    , hasNewEvents(0)
    , friendStatus(Status::Offline)
    , offlineEngine(this)
{
    if (userName.isEmpty())
        userName = UserId.publicKey;
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
        name = userID.publicKey;

    if (userName != name)
    {
        userName = name;
        emit nameChanged(userName);
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
    if (userAlias != alias)
    {
        userAlias = alias;
        emit aliasChanged(friendId, alias);
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
    if (statusMessage != message)
    {
        statusMessage = message;
        emit newStatusMessage(message);
    }
}

/**
 * @brief Get status message.
 * @return Friend status message.
 */
QString Friend::getStatusMessage()
{
    return statusMessage;
}

/**
 * @brief Returns name, which should be displayed.
 * @return Friend displayed name.
 *
 * Return friend alias if setted, username otherwise.
 */
QString Friend::getDisplayedName() const
{
    return userAlias.isEmpty() ? userName : userAlias;
}

/**
 * @brief Checks, if friend has alias.
 * @return True, if user sets alias for this friend, false otherwise.
 */
bool Friend::hasAlias() const
{
    return !userAlias.isEmpty();
}

/**
 * @brief Get ToxId
 * @return ToxId of current friend.
 */
const ToxId &Friend::getToxId() const
{
    return userID;
}

/**
 * @brief Get friend id.
 * @return Return friend id.
 */
uint32_t Friend::getFriendId() const
{
    return friendId;
}

/**
 * @brief Set event flag.
 * @param flag True if friend has new event, false otherwise.
 */
void Friend::setEventFlag(bool flag)
{
    hasNewEvents = flag;
}

/**
 * @brief Get event flag.
 * @return Return true, if friend has new event, false otherwise.
 */
bool Friend::getEventFlag() const
{
    return hasNewEvents;
}

/**
 * @brief Set friend status.
 * @param s New friend status.
 */
void Friend::setStatus(Status s)
{
    if (friendStatus != s)
    {
        friendStatus = s;
        emit statusChanged(friendId, friendStatus);
    }
}

/**
 * @brief Get friend status.
 * @return Status of current friend.
 */
Status Friend::getStatus() const
{
    return friendStatus;
}

/**
 * @brief Returns the friend's @a OfflineMessageEngine.
 * @return a const reference to the offline engine
 */
const OfflineMsgEngine& Friend::getOfflineMsgEngine() const
{
    return offlineEngine;
}

void Friend::registerReceipt(int rec, qint64 id, ChatMessage::Ptr msg)
{
    offlineEngine.registerReceipt(rec, id, msg);
}

void Friend::dischargeReceipt(int receipt)
{
    offlineEngine.dischargeReceipt(receipt);
}

void Friend::clearOfflineReceipts()
{
    offlineEngine.removeAllReceipts();
}

void Friend::deliverOfflineMsgs()
{
    offlineEngine.deliverOfflineMsgs();
}
