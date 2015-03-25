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
#include "widget/gui.h"
#include "src/core.h"
#include "src/misc/settings.h"

Friend::Friend(int FriendId, const ToxID &UserId)
    : userName{Core::getInstance()->getPeerName(UserId)},
      userID{UserId}, friendId{FriendId}
{
    hasNewEvents = 0;
    friendStatus = Status::Offline;
    if (userName.size() == 0)
        userName = UserId.publicKey;

    userAlias = Settings::getInstance().getFriendAlias(UserId);

    widget = new FriendWidget(friendId, getDisplayedName());
    chatForm = new ChatForm(this);
    if (Settings::getInstance().getEnableLogging())
    {
        chatForm->loadHistory(QDateTime::currentDateTime().addDays(-7), true);
        widget->historyLoaded = true;
    }
}

Friend::~Friend()
{
    delete chatForm;
    delete widget;
}

void Friend::setName(QString name)
{
    userName = name;
    if (userAlias.size() == 0)
    {
        widget->setName(name);
        chatForm->setName(name);

        if (widget->isActive())
            GUI::setWindowTitle(name);
        
        emit displayedNameChanged(getFriendWidget(), getStatus(), hasNewEvents);
    }
}

void Friend::setAlias(QString name)
{
    userAlias = name;
    QString dispName = userAlias.size() == 0 ? userName : userAlias;

    widget->setName(dispName);
    chatForm->setName(dispName);

    if (widget->isActive())
            GUI::setWindowTitle(dispName);
    
    emit displayedNameChanged(getFriendWidget(), getStatus(), hasNewEvents);
}

void Friend::setStatusMessage(QString message)
{
    widget->setStatusMsg(message);
    chatForm->setStatusMessage(message);
}

QString Friend::getDisplayedName() const
{
    if (userAlias.size() == 0)
        return userName;
    return userAlias;
}

const ToxID &Friend::getToxID() const
{
    return userID;
}

int Friend::getFriendID() const
{
    return friendId;
}

void Friend::setEventFlag(int f)
{
    hasNewEvents = f;
}

int Friend::getEventFlag() const
{
    return hasNewEvents;
}

void Friend::setStatus(Status s)
{
    friendStatus = s;
}

Status Friend::getStatus() const
{
    return friendStatus;
}

ChatForm *Friend::getChatForm()
{
    return chatForm;
}

FriendWidget *Friend::getFriendWidget()
{
    return widget;
}
