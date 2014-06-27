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
