#ifndef FRIEND_H
#define FRIEND_H

#include <QString>
#include "chatform.h"

class FriendWidget;

struct Friend
{
public:
    Friend(int FriendId, QString UserId);
    ~Friend();
    void setName(QString name);
    void setStatusMessage(QString message);
    QString getName();

public:
    FriendWidget* widget;
    int friendId;
    QString userId;
    ChatForm* chatForm;
};

#endif // FRIEND_H
