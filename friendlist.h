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

#include <QString>
#include <QList>

struct Friend;

class FriendList
{
public:
    FriendList();
    static Friend* addFriend(int friendId, QString userId);
    static Friend* findFriend(int friendId);
    static void removeFriend(int friendId);

public:
    static QList<Friend*> friendList;
};

#endif // FRIENDLIST_H
