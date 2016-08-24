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
#include "widget/form/chatform.h"
#include "src/core/core.h"
#include "src/persistence/settings.h"
#include "src/persistence/profile.h"
#include "src/nexus.h"
#include "src/grouplist.h"
#include "src/group.h"

Friend::Friend(uint32_t FriendId, const ToxId &UserId)
    : userName(Core::getInstance()->getPeerName(UserId))
    , userAlias(Settings::getInstance().getFriendAlias(UserId))
    , userID(UserId)
    , friendId(FriendId)
    , hasNewEvents(0)
    , friendStatus(Status::Offline)
{
    if (userName.isEmpty())
        userName = UserId.publicKey;

    chatForm = new ChatForm(this);
}

Friend::~Friend()
{
    delete chatForm;
}

void Friend::setName(QString name)
{
    if (name.isEmpty())
    {
        name = userID.publicKey;
    }

    if (userName != name && userAlias.isEmpty())
    {
        userName = name;
        chatForm->setName(name);
        emit nameChanged(friendId, userName);
    }
}

void Friend::setAlias(const QString& alias)
{
    if (userAlias != alias)
    {
        userAlias = alias;
        chatForm->setName(alias);
        emit aliasChanged(friendId, alias);
    }
}

void Friend::setStatusMessage(const QString& message)
{
    if (statusMessage != message)
    {
        statusMessage = message;
        chatForm->setStatusMessage(message);
        emit newStatusMessage(message);
    }
}

QString Friend::getStatusMessage()
{
    return statusMessage;
}

QString Friend::getDisplayedName() const
{
    if (userAlias.size() == 0)
        return userName;

    return userAlias;
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

void Friend::setEventFlag(int flag)
{
    hasNewEvents = flag;
}

int Friend::getEventFlag() const
{
    return hasNewEvents;
}

void Friend::setStatus(Status s)
{
    if (friendStatus != s)
    {
        friendStatus = s;
        emit statusChanged(friendId, friendStatus);
    }
}

Status Friend::getStatus() const
{
    return friendStatus;
}

ChatForm* Friend::getChatForm()
{
    return chatForm;
}
