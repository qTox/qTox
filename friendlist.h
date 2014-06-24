#ifndef FRIENDLIST_H
#define FRIENDLIST_H

#include <QString>
#include <QList>

class Friend;

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
