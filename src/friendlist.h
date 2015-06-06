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

#ifndef FRIENDLIST_H
#define FRIENDLIST_H

template <class T> class QList;
template <class A, class B> class QHash;
class Friend;
class QString;
class ToxId;

class FriendList
{
public:
    static Friend* addFriend(int friendId, const ToxId &userId);
    static Friend* findFriend(int friendId);
    static Friend* findFriend(const ToxId &userId);
    static QList<Friend*> getAllFriends();
    static void removeFriend(int friendId, bool fake = false);
    static void clear();

private:
    static QHash<int, Friend*> friendList;
    static QHash<QString, int> tox2id;
};

#endif // FRIENDLIST_H
