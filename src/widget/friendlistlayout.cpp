/*
    Copyright © 2019 by The qTox Project Contributors

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

#include "friendlistlayout.h"
#include "friendlistwidget.h"
#include "friendwidget.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/friendlist.h"
#include <QDebug>
#include <cassert>

FriendListLayout::FriendListLayout()
    : QVBoxLayout()
{
    init();
}

FriendListLayout::FriendListLayout(QWidget* parent)
    : QVBoxLayout(parent)
{
    init();
}

void FriendListLayout::init()
{
    setSpacing(0);
    setMargin(0);

    friendOnlineLayout.getLayout()->setSpacing(0);
    friendOnlineLayout.getLayout()->setMargin(0);

    friendOfflineLayout.getLayout()->setSpacing(0);
    friendOfflineLayout.getLayout()->setMargin(0);

    friendBlockedLayout.getLayout()->setSpacing(0);
    friendBlockedLayout.getLayout()->setMargin(0);

    addLayout(friendOnlineLayout.getLayout());
    addLayout(friendOfflineLayout.getLayout());
    addLayout(friendBlockedLayout.getLayout());
}

void FriendListLayout::addFriendWidget(FriendWidget* w, Status::Status s)
{
    friendOfflineLayout.removeSortedWidget(w);
    friendOnlineLayout.removeSortedWidget(w);
    friendBlockedLayout.removeSortedWidget(w);

    if (s == Status::Status::Offline) {
        friendOfflineLayout.addSortedWidget(w);
    } else if (s == Status::Status::Blocked) {
        friendBlockedLayout.addSortedWidget(w);
    } else {
        friendOnlineLayout.addSortedWidget(w);
    }
}

void FriendListLayout::removeFriendWidget(FriendWidget* widget, Status::Status s)
{
    if (s == Status::Status::Offline) {
        friendOfflineLayout.removeSortedWidget(widget);
    } else if (s == Status::Status::Blocked) {
        friendBlockedLayout.removeSortedWidget(widget);
    } else {
        friendOnlineLayout.removeSortedWidget(widget);
    }
}

int FriendListLayout::indexOfFriendWidget(GenericChatItemWidget* widget, Status::Status s) const
{
    if (s == Status::Status::Offline) {
        return friendOfflineLayout.indexOfSortedWidget(widget);
    } else if (s == Status::Status::Blocked) {
        return friendBlockedLayout.indexOfSortedWidget(widget);
    } else {
        return friendOnlineLayout.indexOfSortedWidget(widget);
    }
}

void FriendListLayout::moveFriendWidgets(FriendListWidget* listWidget)
{
    while (!friendOnlineLayout.getLayout()->isEmpty()) {
        QWidget* getWidget = friendOnlineLayout.getLayout()->takeAt(0)->widget();

        FriendWidget* friendWidget = qobject_cast<FriendWidget*>(getWidget);
        const Friend* f = friendWidget->getFriend();
        listWidget->moveWidget(friendWidget, f->getStatus(), true);
    }
    while (!friendOfflineLayout.getLayout()->isEmpty()) {
        QWidget* getWidget = friendOfflineLayout.getLayout()->takeAt(0)->widget();

        FriendWidget* friendWidget = qobject_cast<FriendWidget*>(getWidget);
        const Friend* f = friendWidget->getFriend();
        listWidget->moveWidget(friendWidget, f->getStatus(), true);
    }
    while (!friendBlockedLayout.getLayout()->isEmpty()) {
        QWidget* getWidget = friendBlockedLayout.getLayout()->takeAt(0)->widget();

        FriendWidget* friendWidget = qobject_cast<FriendWidget*>(getWidget);
        const Friend* f = friendWidget->getFriend();
        listWidget->moveWidget(friendWidget, f->getStatus(), true);
    }
}

int FriendListLayout::friendOnlineCount() const
{
    return friendOnlineLayout.getLayout()->count();
}

int FriendListLayout::friendTotalCount() const
{
    return friendBlockedLayout.getLayout()->count()
        + friendOfflineLayout.getLayout()->count()
        + friendOnlineCount();
}

bool FriendListLayout::hasChatrooms() const
{
    return !(friendOfflineLayout.getLayout()->isEmpty()
        && friendOnlineLayout.getLayout()->isEmpty()
        && friendBlockedLayout.getLayout()->isEmpty());
}

void FriendListLayout::searchChatrooms(const QString& searchString, bool hideOnline, bool hideOffline, bool hideBlocked)
{
    friendOnlineLayout.search(searchString, hideOnline);
    friendOfflineLayout.search(searchString, hideOffline);
    friendBlockedLayout.search(searchString, hideBlocked);
}

QLayout* FriendListLayout::getLayoutOnline() const
{
    return friendOnlineLayout.getLayout();
}

QLayout* FriendListLayout::getLayoutOffline() const
{
    return friendOfflineLayout.getLayout();
}

QLayout* FriendListLayout::getLayoutBlocked() const
{
    return friendBlockedLayout.getLayout();
}

QLayout* FriendListLayout::getFriendLayout(Status::Status s) const
{
    if (s == Status::Status::Offline) {
        return friendOfflineLayout.getLayout();
    } else if (s == Status::Status::Blocked) {
        return friendBlockedLayout.getLayout();
    } else {
        return friendOnlineLayout.getLayout();
    }
}
