/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef FRIENDLIST_H
#define FRIENDLIST_H

template <class T> class QList;
template <class A, class B> class QHash;
class Friend;
class QString;
struct ToxID;

class FriendList
{
public:
    static Friend* addFriend(int friendId, const ToxID &userId);
    static Friend* findFriend(int friendId);
    static Friend* findFriend(const ToxID &userId);
    static QList<Friend*> getAllFriends();
    static void removeFriend(int friendId, bool fake = false);
    static void clear();

private:
    static QHash<int, Friend*> friendList;
    static QHash<QString, int> tox2id;
};

#endif // FRIENDLIST_H
