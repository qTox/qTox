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
