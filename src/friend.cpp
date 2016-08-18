/*
    Copyright © 2014-2015 by The qTox Project

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
#include "friendlist.h"
#include "widget/friendwidget.h"
#include "widget/gui.h"
#include "src/core/core.h"
#include "src/persistence/settings.h"
#include "src/persistence/profile.h"
#include "src/nexus.h"
#include "src/grouplist.h"
#include "src/group.h"

Friend::Friend(uint32_t FriendId, const ToxId &UserId)
    : userName{Core::getInstance()->getPeerName(UserId)}
    , userAlias(Settings::getInstance().getFriendAlias(UserId))
    , userID(UserId)
    , friendId(FriendId)
    , hasNewEvents(0)
    , friendStatus(Status::Offline)
    , widget(new FriendWidget(friendId, getDisplayedName()))
    , offlineEngine(this)
{
    if (userName.isEmpty())
        userName = UserId.publicKey;
}

Friend::~Friend()
{
    delete widget;
}

/**
 * @brief Loads the friend's chat history if enabled
 */
void Friend::loadHistory()
{
    if (Nexus::getProfile()->isHistoryEnabled())
    {
        emit loadChatHistory();
    }
}

void Friend::setName(QString name)
{
    if (name.isEmpty())
        name = userID.publicKey;

    if (name != userName)
    {
        userName = name;
        emit nameChanged(userName);
    }

    // TODO: the following is old code -> refactor/remove
    if (userAlias.isEmpty())
    {
        widget->setName(name);

        if (widget->isActive())
            GUI::setWindowTitle(name);

        emit displayedNameChanged(getFriendWidget(), getStatus(), hasNewEvents);
    }
}

void Friend::setAlias(QString name)
{
    userAlias = name;
    QString dispName = userAlias.isEmpty() ? userName : userAlias;

    widget->setName(dispName);

    if (widget->isActive())
            GUI::setWindowTitle(dispName);

    emit displayedNameChanged(getFriendWidget(), getStatus(), hasNewEvents);

    for (Group *g : GroupList::getAllGroups())
    {
        g->regeneratePeerList();
    }
}

void Friend::setStatusMessage(QString message)
{
    statusMessage = message;
    // TODO: connect FriendWidget to signal
    widget->setStatusMsg(message);
    emit newStatusMessage(message);
}

QString Friend::getStatusMessage()
{
    return statusMessage;
}

QString Friend::getDisplayedName() const
{
    return userAlias.isEmpty() ? userName : userAlias;
}

bool Friend::hasAlias() const
{
    return !userAlias.isEmpty();
}

const ToxId &Friend::getToxId() const
{
    return userID;
}

uint32_t Friend::getFriendId() const
{
    return friendId;
}

void Friend::setEventFlag(int f)
{
    hasNewEvents = f;
}

int Friend::getEventFlag() const
{
    return hasNewEvents;
}

void Friend::setStatus(Status s)
{
    if (s != friendStatus)
    {
        friendStatus = s;
        emit statusChanged(friendId, friendStatus);
    }
}

Status Friend::getStatus() const
{
    return friendStatus;
}

void Friend::setFriendWidget(FriendWidget *widget)
{
    this->widget = widget;
}

FriendWidget *Friend::getFriendWidget()
{
    return widget;
}

const FriendWidget *Friend::getFriendWidget() const
{
    return widget;
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
