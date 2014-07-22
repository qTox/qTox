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

Friend::Friend(int FriendId, QString UserId)
    : friendId(FriendId), userId(UserId)
{
    widget = new FriendWidget(friendId, userId);
    chatForm = new ChatForm(this);
    hasNewMessages = 0;
    friendStatus = Status::Offline;
}

Friend::~Friend()
{
    delete chatForm;
    delete widget;
}

void Friend::setName(QString name)
{
    widget->name.setText(name);
    widget->name.setToolTip(name); // for overlength names
    chatForm->setName(name);
}

void Friend::setStatusMessage(QString message)
{
    widget->statusMessage.setText(message);
    widget->statusMessage.setToolTip(message); // for overlength messsages
    chatForm->setStatusMessage(message);
}

QString Friend::getName()
{
    return widget->name.text();
}
