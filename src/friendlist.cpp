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

#include "friendlist.h"
#include "src/model/friend.h"
#include "src/persistence/settings.h"
#include <QDebug>
#include <QHash>
#include <QMenu>

QHash<int, Friend*> FriendList::friendList;
QHash<QByteArray, int> FriendList::key2id;

Friend* FriendList::addFriend(int friendId, const ToxPk& friendPk)
{
    auto friendChecker = friendList.find(friendId);
    if (friendChecker != friendList.end())
        qWarning() << "addFriend: friendId already taken";

    QString alias = Settings::getInstance().getFriendAlias(friendPk);
    Friend* newfriend = new Friend(friendId, friendPk, alias);
    friendList[friendId] = newfriend;
    key2id[friendPk.getKey()] = friendId;

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
    if (f_it != friendList.end()) {
        if (!fake)
            Settings::getInstance().removeFriendSettings(f_it.value()->getPublicKey());
        friendList.erase(f_it);
    }
}

void FriendList::clear()
{
    for (auto friendptr : friendList)
        delete friendptr;
    friendList.clear();
}

Friend* FriendList::findFriend(const ToxPk& friendPk)
{
    auto id = key2id.find(friendPk.getKey());
    if (id != key2id.end()) {
        Friend* f = findFriend(*id);
        if (!f)
            return nullptr;
        if (f->getPublicKey() == friendPk)
            return f;
    }

    return nullptr;
}

QList<Friend*> FriendList::getAllFriends()
{
    return friendList.values();
}
