/*
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

#include <cstdint>

#include <QHash>

template <class T>
class QList;
class Friend;
class QByteArray;
class QString;
class ToxPk;
class Settings;

class FriendList
{
public:
    Friend* addFriend(uint32_t friendId, const ToxPk& friendPk, Settings& settings);
    Friend* findFriend(const ToxPk& friendPk);
    const ToxPk& id2Key(uint32_t friendId);
    QList<Friend*> getAllFriends();
    void removeFriend(const ToxPk& friendPk, Settings& settings, bool fake = false);
    void clear();
    QString decideNickname(const ToxPk& friendPk, const QString& origName);

private:
    QHash<ToxPk, Friend*> friendList;
    QHash<uint32_t, ToxPk> id2key;
};
