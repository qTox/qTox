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
#include "widget/friendwidget.h"
#include "widget/form/chatform.h"

Friend::Friend(int FriendId, QString UserId)
    : friendId(FriendId), userId(UserId)
{
    widget = new FriendWidget(friendId, userId);
    chatForm = new ChatForm(this);
    hasNewEvents = 0;
    friendStatus = Status::Offline;
}

Friend::~Friend()
{
    delete chatForm;
    delete widget;
}

void Friend::setName(QString name)
{
    widget->setName(name);
    chatForm->setName(name);
}

void Friend::setStatusMessage(QString message)
{
    widget->setStatusMsg(message);
    chatForm->setStatusMessage(message);
}

QString Friend::getName()
{
    return widget->getName();
}
