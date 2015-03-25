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

#include <QObject>
#include <QString>
#include "corestructs.h"

struct FriendWidget;
class ChatForm;

struct Friend : QObject
{
    Q_OBJECT
public:
    Friend(int FriendId, const ToxID &UserId);
    Friend(const Friend& other)=delete;
    ~Friend();
    Friend& operator=(const Friend& other)=delete;

    void setName(QString name);
    void setAlias(QString name);
    QString getDisplayedName() const;

    void setStatusMessage(QString message);

    void setEventFlag(int f);
    int getEventFlag() const;

    const ToxID &getToxID() const;
    int getFriendID() const;

    void setStatus(Status s);
    Status getStatus() const;

    ChatForm *getChatForm();
    FriendWidget *getFriendWidget();

signals:
    void displayedNameChanged(FriendWidget* widget, Status s, int hasNewEvents);

private:
    QString userAlias, userName;
    ToxID userID;
    int friendId;
    int hasNewEvents;
    Status friendStatus;

    FriendWidget* widget;
    ChatForm* chatForm;
};

#endif // FRIEND_H
