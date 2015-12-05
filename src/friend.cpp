/*
    Copyright © 2014-2015 by The qTox Project

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


#include "friend.h"
#include "friendlist.h"
#include "widget/friendwidget.h"
#include "widget/form/chatform.h"
#include "widget/gui.h"
#include "src/core/core.h"
#include "src/persistence/settings.h"
#include "src/persistence/historykeeper.h"

Friend::Friend(uint32_t FriendId, const ToxId &UserId)
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
    HistoryKeeper::getInstance()->importAvatarToDatabase(UserId.publicKey);
}

Friend::~Friend()
{
    delete chatForm;
    delete widget;
}

void Friend::loadHistory()
{
    if (Settings::getInstance().getEnableLogging())
    {
        chatForm->loadHistory(QDateTime::currentDateTime().addDays(-7), true);
        widget->historyLoaded = true;
    }
}

void Friend::setName(QString name)
{
   if (name.isEmpty())
       name = userID.publicKey;

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
    statusMessage = message;
    widget->setStatusMsg(message);
    chatForm->setStatusMessage(message);
}

QString Friend::getStatusMessage()
{
    return statusMessage;
}

QString Friend::getDisplayedName() const
{
    if (userAlias.size() == 0)
        return userName;

    return userAlias;
}

bool Friend::hasAlias() const
{
    return !userAlias.isEmpty();
}

const ToxId &Friend::getToxId() const
{
    return userID;
}

uint32_t Friend::getFriendID() const
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

const FriendWidget *Friend::getFriendWidget() const
{
    return widget;
}
