#ifndef FRIEND_H
#define FRIEND_H

#include <QString>
#include "widget/form/chatform.h"

struct FriendWidget;

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
    int hasNewMessages;
    Status friendStatus;
    QPixmap avatar;
};

#endif // FRIEND_H
