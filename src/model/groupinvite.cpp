/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

#include "groupinvite.h"

/**
 * @class GroupInvite
 *
 * @brief This class contains information needed to create a group invite
 */

GroupInvite::GroupInvite(uint32_t friendId_, uint8_t inviteType, const QByteArray& data)
    : friendId{friendId_}
    , type{inviteType}
    , invite{data}
    , date{QDateTime::currentDateTime()}
{
}

bool GroupInvite::operator==(const GroupInvite& other) const
{
    return friendId == other.friendId && type == other.type && invite == other.invite
           && date == other.date;
}

uint32_t GroupInvite::getFriendId() const
{
    return friendId;
}

uint8_t GroupInvite::getType() const
{
    return type;
}

QByteArray GroupInvite::getInvite() const
{
    return invite;
}

QDateTime GroupInvite::getInviteDate() const
{
    return date;
}
