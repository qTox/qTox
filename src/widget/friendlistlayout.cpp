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
#include <cassert>

#include "groupwidget.h"
#include "friendwidget.h"

#include <QDebug>

FriendListLayout::FriendListLayout(bool groupsOnTop)
{
    setSpacing(0);
    setMargin(0);

    groupLayout = new QVBoxLayout();
    groupLayout->setSpacing(0);
    groupLayout->setMargin(0);

    friendLayouts[Online] = new QVBoxLayout();
    friendLayouts[Online]->setSpacing(0);
    friendLayouts[Online]->setMargin(0);

    friendLayouts[Offline] = new QVBoxLayout();
    friendLayouts[Offline]->setSpacing(0);
    friendLayouts[Offline]->setMargin(0);

    if (groupsOnTop)
    {
        QVBoxLayout::addLayout(groupLayout);
        QVBoxLayout::addLayout(friendLayouts[Online]);
        QVBoxLayout::addLayout(friendLayouts[Offline]);
    }
    else
    {
        QVBoxLayout::addLayout(friendLayouts[Online]);
        QVBoxLayout::addLayout(groupLayout);
        QVBoxLayout::addLayout(friendLayouts[Offline]);
    }
}

void FriendListLayout::addFriendWidget(FriendWidget *w, Status s)
{
    QVBoxLayout* l = getFriendLayout(s);
    l->removeWidget(w); // In case the widget is already in this layout.
    Friend* g = FriendList::findFriend(w->friendId);

    // Binary search.
    int min = 0, max = l->count(), mid;
    while (min < max)
    {
        mid = (max - min) / 2 + min;
        FriendWidget* w1 = dynamic_cast<FriendWidget*>(l->itemAt(mid)->widget());
        assert(w1 != nullptr);

        Friend* f = FriendList::findFriend(w1->friendId);
        int compareValue = f->getDisplayedName().localeAwareCompare(g->getDisplayedName());
        if (compareValue > 0)
        {
            max = mid;
        }
        else
        {
            min = mid + 1;
        }
    }

    l->insertWidget(min, w);
}

int FriendListLayout::friendOnlineCount() const
{
    return friendLayouts[Online]->count();
}

int FriendListLayout::friendOfflineCount() const
{
    return friendLayouts[Offline]->count();
}

template <typename WidgetType>
void searchHelper(const QString &searchString, QBoxLayout *boxLayout, bool hideAll)
{
    for (int index = 0; index < boxLayout->count(); ++index)
    {
        WidgetType* widgetAt = static_cast<WidgetType*>(boxLayout->itemAt(index)->widget());
        QString widgetName = widgetAt->getName();

        widgetAt->setVisible(!hideAll && widgetName.contains(searchString, Qt::CaseInsensitive));
    }
}

void FriendListLayout::searchChatrooms(const QString &searchString, bool hideOnline, bool hideOffline, bool hideGroups)
{
    searchHelper<GroupWidget>(searchString, groupLayout, hideGroups);
    searchHelper<FriendWidget>(searchString, friendLayouts[Online], hideOnline);
    searchHelper<FriendWidget>(searchString, friendLayouts[Offline], hideOffline);
}

bool FriendListLayout::hasChatrooms() const
{
    return !groupLayout->isEmpty() || !friendLayouts[Online]->isEmpty() || !friendLayouts[Offline]->isEmpty();
}

QVBoxLayout* FriendListLayout::getFriendLayout(Status s)
{
    return s == Status::Offline ? friendLayouts[Offline] : friendLayouts[Online];
}
