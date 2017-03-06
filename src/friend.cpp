/*
    Copyright © 2014-2015 by The qTox Project Contributors

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
#include "widget/form/chatform.h"
#include "widget/friendwidget.h"
#include "widget/gui.h"
#include "src/core/core.h"
#include "src/persistence/settings.h"
#include "src/persistence/profile.h"
#include "src/nexus.h"
#include "src/grouplist.h"
#include "src/group.h"

Friend::Friend(uint32_t friendId, const ToxPk& friendPk)
    : userName{Core::getInstance()->getPeerName(friendPk)}
    , userAlias(Settings::getInstance().getFriendAlias(friendPk))
    , friendPk(friendPk)
    , friendId(friendId)
    , hasNewEvents(false)
    , friendStatus(Status::Offline)
{
    if (userName.isEmpty())
    {
        userName = friendPk.toString();
    }

    userAlias = Settings::getInstance().getFriendAlias(friendPk);

    chatForm = new ChatForm(this);
}

Friend::~Friend()
{
    delete chatForm;
    delete widget;
}

/**
 * @brief Loads the friend's chat history if enabled
 */
void Friend::loadHistory()
{
    if (Nexus::getProfile()->isHistoryEnabled())
    {
        chatForm->loadHistory(QDateTime::currentDateTime().addDays(-7), true);
        widget->historyLoaded = true;
    }
}

void Friend::setName(QString name)
{
   if (name.isEmpty())
       name = friendPk.toString();

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
    QString dispName = userAlias.isEmpty() ? userName : userAlias;

    widget->setName(dispName);
    chatForm->setName(dispName);

    if (widget->isActive())
            GUI::setWindowTitle(dispName);

    emit displayedNameChanged(getFriendWidget(), getStatus(), hasNewEvents);

    for (Group *g : GroupList::getAllGroups())
    {
        g->regeneratePeerList();
    }
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
    return userAlias.isEmpty() ? userName : userAlias;
}

bool Friend::hasAlias() const
{
    return !userAlias.isEmpty();
}

const ToxPk& Friend::getPublicKey() const
{
    return friendPk;
}

uint32_t Friend::getFriendId() const
{
    return friendId;
}

void Friend::setEventFlag(bool flag)
{
    hasNewEvents = flag;
}

bool Friend::getEventFlag() const
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

void Friend::setFriendWidget(FriendWidget *widget)
{
    this->widget = widget;
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
