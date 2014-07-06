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

#include "friend.h"
#include "friendlist.h"
#include <QMenu>

QList<Friend*> FriendList::friendList;

Friend* FriendList::addFriend(int friendId, QString userId)
{
    for (Friend* f : friendList)
        if (f->friendId == friendId)
            qWarning() << "FriendList::addFriend: friendId already taken";
    Friend* newfriend = new Friend(friendId, userId);
    friendList.append(newfriend);
    return newfriend;
}

Friend* FriendList::findFriend(int friendId)
{
    for (Friend* f : friendList)
        if (f->friendId == friendId)
            return f;
    return nullptr;
}

void FriendList::removeFriend(int friendId)
{
    for (int i=0; i<friendList.size(); i++)
    {
        if (friendList[i]->friendId == friendId)
        {
            friendList.removeAt(i);
            return;
        }
    }
}
