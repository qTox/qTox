/*
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

#include "friendlistlayout.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "friendwidget.h"
#include "friendlistwidget.h"
#include <cassert>

FriendListLayout::FriendListLayout()
{
    setSpacing(0);
    setMargin(0);

    friendOnlineLayout.getLayout()->setSpacing(0);
    friendOnlineLayout.getLayout()->setMargin(0);

    friendOfflineLayout.getLayout()->setSpacing(0);
    friendOfflineLayout.getLayout()->setMargin(0);

    addLayout(friendOnlineLayout.getLayout());
    addLayout(friendOfflineLayout.getLayout());
}

FriendListLayout::FriendListLayout(QWidget* parent)
    : QVBoxLayout(parent)
{
    FriendListLayout();
}

void FriendListLayout::addFriendWidget(FriendWidget* w, Status s)
{
    friendOfflineLayout.removeSortedWidget(w);
    friendOnlineLayout.removeSortedWidget(w);

    if (s == Status::Offline)
    {
        friendOfflineLayout.addSortedWidget(w);
        return;
    }

    friendOnlineLayout.addSortedWidget(w);
}

void FriendListLayout::removeFriendWidget(FriendWidget *widget, Status s)
{
    if (s == Status::Offline)
        friendOfflineLayout.removeSortedWidget(widget);
    else
        friendOnlineLayout.removeSortedWidget(widget);
}

int FriendListLayout::indexOfFriendWidget(GenericChatItemWidget* widget, bool online) const
{
    if (online)
        return friendOnlineLayout.indexOfSortedWidget(widget);
    return friendOfflineLayout.indexOfSortedWidget(widget);
}

void FriendListLayout::moveFriendWidgets(FriendListWidget* listWidget)
{
    while (friendOnlineLayout.getLayout()->count() != 0)
    {
        QWidget* getWidget = friendOnlineLayout.getLayout()->takeAt(0)->widget();

        FriendWidget* friendWidget = dynamic_cast<FriendWidget*>(getWidget);
        listWidget->moveWidget(friendWidget, FriendList::findFriend(friendWidget->friendId)->getStatus(), true);
    }
    while (friendOfflineLayout.getLayout()->count() != 0)
    {
        QWidget* getWidget = friendOfflineLayout.getLayout()->takeAt(0)->widget();

        FriendWidget* friendWidget = dynamic_cast<FriendWidget*>(getWidget);
        listWidget->moveWidget(friendWidget, FriendList::findFriend(friendWidget->friendId)->getStatus(), true);
    }
}

int FriendListLayout::friendOnlineCount() const
{
    return friendOnlineLayout.getLayout()->count();
}

int FriendListLayout::friendTotalCount() const
{
    return friendOfflineLayout.getLayout()->count() + friendOnlineCount();
}

bool FriendListLayout::hasChatrooms() const
{
    return !(friendOfflineLayout.getLayout()->isEmpty() && friendOnlineLayout.getLayout()->isEmpty());
}

void FriendListLayout::searchChatrooms(const QString& searchString, bool hideOnline, bool hideOffline)
{
    friendOnlineLayout.search(searchString, hideOnline);
    friendOfflineLayout.search(searchString, hideOffline);
}

QLayout* FriendListLayout::getLayoutOnline() const
{
    return friendOnlineLayout.getLayout();
}

QLayout* FriendListLayout::getLayoutOffline() const
{
    return friendOfflineLayout.getLayout();
}

QLayout* FriendListLayout::getFriendLayout(Status s) const
{
    return s == Status::Offline ? friendOfflineLayout.getLayout() : friendOnlineLayout.getLayout();
}
