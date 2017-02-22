/*
    Copyright © 2017 by The qTox Project Contributors

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

#ifndef GROUPINVITE_H
#define GROUPINVITE_H

#include <cstdint>
#include <QByteArray>
#include <QDateTime>

class GroupInvite
{
public:
    GroupInvite(const int32_t FriendId, const uint8_t Type, const QByteArray& Invite);
    bool operator ==(const GroupInvite& other) const;

    int32_t getFriendId() const;
    uint8_t getType() const;
    QByteArray getInvite() const;
    QDateTime getInviteDate() const;

private:
    int32_t friendId;
    uint8_t type;
    QByteArray invite;
    QDateTime date;
};

#endif // GROUPINVITE_H
