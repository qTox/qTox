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
#include "src/misc/settings.h"
#include <QMenu>
#include <QDebug>
#include <QHash>

QHash<int, Friend*> FriendList::friendList;
QHash<QString, int> FriendList::tox2id;

Friend* FriendList::addFriend(int friendId, const ToxID& userId)
{
    auto friendChecker = friendList.find(friendId);
    if (friendChecker != friendList.end())
        qWarning() << "FriendList::addFriend: friendId already taken";

    Friend* newfriend = new Friend(friendId, userId);
    friendList[friendId] = newfriend;
    tox2id[userId.publicKey] = friendId;

    return newfriend;
}

Friend* FriendList::findFriend(int friendId)
{
    auto f_it = friendList.find(friendId);
    if (f_it != friendList.end())
        return *f_it;

    return nullptr;
}

void FriendList::removeFriend(int friendId, bool fake)
{
    auto f_it = friendList.find(friendId);
    if (f_it != friendList.end())
    {
        if (!fake)
            Settings::getInstance().removeFriendSettings(f_it.value()->getToxID());
        friendList.erase(f_it);
    }
}

void FriendList::clear()
{
    for (auto friendptr : friendList)
        delete friendptr;
    friendList.clear();
}

Friend* FriendList::findFriend(const ToxID& userId)
{
    auto id = tox2id.find(userId.publicKey);
    if (id != tox2id.end())
    {
        Friend *f = findFriend(*id);
        if (!f)
            return nullptr;
        if (f->getToxID() == userId)
            return f;
    }

    return nullptr;
}

QList<Friend*> FriendList::getAllFriends()
{
    return friendList.values();
}
