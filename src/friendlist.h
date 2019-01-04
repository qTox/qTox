/*
    Copyright © 2014-2018 by The qTox Project Contributors

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

#ifndef FRIENDLIST_H
#define FRIENDLIST_H

#include <cstdint>

template <class T>
class QList;
template <class A, class B>
class QHash;
class Friend;
class QByteArray;
class QString;
class ToxPk;

class FriendList
{
public:
    static Friend* addFriend(uint32_t friendId, const ToxPk& friendPk);
    static Friend* findFriend(uint32_t friendId);
    static Friend* findFriend(const ToxPk& friendPk);
    static QList<Friend*> getAllFriends();
    static void removeFriend(uint32_t friendId, bool fake = false);
    static void clear();
    static QString decideNickname(ToxPk peerPk, const QString origName); 

private:
    static QHash<uint32_t, Friend*> friendList;
    static QHash<QByteArray, uint32_t> key2id;
};

#endif // FRIENDLIST_H
