/*
    Copyright © 2014-2015 by The qTox Project Contributors

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
#include "src/model/group.h"
#include "src/grouplist.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/widget/form/chatform.h"

Friend::Friend(uint32_t friendId, const ToxPk& friendPk, const QString& userAlias)
    : userName{Core::getInstance()->getPeerName(friendPk)}
    , userAlias{userAlias}
    , friendPk{friendPk}
    , friendId{friendId}
    , hasNewEvents{false}
    , friendStatus{Status::Offline}
{
    if (userName.isEmpty()) {
        userName = friendPk.toString();
    }
}

void Friend::setName(const QString& _name)
{
    QString name = _name;
    if (name.isEmpty()) {
        name = friendPk.toString();
    }

    if (userName != name) {
        userName = name;
        emit nameChanged(friendId, name);
    }
}

void Friend::setAlias(const QString& alias)
{
    if (userAlias != alias) {
        userAlias = alias;
        emit aliasChanged(friendId, alias);
    }
}

void Friend::setStatusMessage(const QString& message)
{
    if (statusMessage != message) {
        statusMessage = message;
        emit statusMessageChanged(friendId, message);
    }
}

QString Friend::getStatusMessage() const
{
    return statusMessage;
}

QString Friend::getDisplayedName() const
{
    if (userAlias.isEmpty()) {
        return userName;
    }

    return userAlias;
}

bool Friend::hasAlias() const
{
    return !userAlias.isEmpty();
}

const ToxPk& Friend::getPublicKey() const
{
    return friendPk;
}

uint32_t Friend::getId() const
{
    return friendId;
}

void Friend::setEventFlag(bool flag)
{
    hasNewEvents = flag;
}

bool Friend::getEventFlag() const
{
    return hasNewEvents;
}

void Friend::setStatus(Status s)
{
    if (friendStatus != s) {
        friendStatus = s;
        emit statusChanged(friendId, friendStatus);
    }
}

Status Friend::getStatus() const
{
    return friendStatus;
}
