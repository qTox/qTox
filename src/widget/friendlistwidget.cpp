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
#include "friendlistwidget.h"
#include <QDebug>
#include <QGridLayout>
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/widget/friendwidget.h"
#include "groupwidget.h"
#include "circlewidget.hpp"
#include <cassert>

FriendListWidget::FriendListWidget(QWidget *parent, bool groupchatPosition) :
    QWidget(parent)
{
    mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    layout()->setSpacing(0);
    layout()->setMargin(0);

    groupLayout = new QVBoxLayout();
    groupLayout->setSpacing(0);
    groupLayout->setMargin(0);

    friendLayouts[Online] = new QVBoxLayout();
    friendLayouts[Online]->setSpacing(0);
    friendLayouts[Online]->setMargin(0);

    friendLayouts[Offline] = new QVBoxLayout();
    friendLayouts[Offline]->setSpacing(0);
    friendLayouts[Offline]->setMargin(0);

    circleLayout = new QVBoxLayout();
    circleLayout->setSpacing(0);
    circleLayout->setMargin(0);

    if (groupchatPosition)
    {
        mainLayout->addLayout(groupLayout);
        mainLayout->addLayout(friendLayouts[Online]);
        mainLayout->addLayout(friendLayouts[Offline]);
    }
    else
    {
        mainLayout->addLayout(friendLayouts[Online]);
        mainLayout->addLayout(groupLayout);
        mainLayout->addLayout(friendLayouts[Offline]);
    }
    mainLayout->addLayout(circleLayout);
}

void FriendListWidget::addGroupWidget(GroupWidget *widget)
{
    groupLayout->addWidget(widget);
}

void FriendListWidget::hideGroups(QString searchString, bool hideAll)
{
    QVBoxLayout* groups = groupLayout;
    int groupCount = groups->count(), index;

    for (index = 0; index<groupCount; index++)
    {
        GroupWidget* groupWidget = static_cast<GroupWidget*>(groups->itemAt(index)->widget());
        QString groupName = groupWidget->getName();

        if (!groupName.contains(searchString, Qt::CaseInsensitive) | hideAll)
            groupWidget->setVisible(false);
        else
            groupWidget->setVisible(true);
    }
}

void FriendListWidget::addCircleWidget(CircleWidget *widget)
{
    circleLayout->addWidget(widget);
}

void FriendListWidget::hideFriends(QString searchString, Status status, bool hideAll)
{
    QVBoxLayout* friends = getFriendLayout(status);
    int friendCount = friends->count(), index;

    for (index = 0; index<friendCount; index++)
    {
        FriendWidget* friendWidget = static_cast<FriendWidget*>(friends->itemAt(index)->widget());
        QString friendName = friendWidget->getName();

        if (!friendName.contains(searchString, Qt::CaseInsensitive) | hideAll)
            friendWidget->setVisible(false);
        else
            friendWidget->setVisible(true);
    }
}

QVBoxLayout* FriendListWidget::getFriendLayout(Status s)
{
    if (s == Status::Offline)
    {
        return friendLayouts[Offline];
    }
    return friendLayouts[Online];
}

void FriendListWidget::onGroupchatPositionChanged(bool top)
{
    mainLayout->removeItem(circleLayout);
    mainLayout->removeItem(groupLayout);
    mainLayout->removeItem(getFriendLayout(Status::Online));
    if (top)
    {
        mainLayout->addLayout(groupLayout);
        mainLayout->addLayout(friendLayouts[Online]);
    }
    else
    {
        mainLayout->addLayout(friendLayouts[Online]);
        mainLayout->addLayout(groupLayout);
    }
    mainLayout->addLayout(circleLayout);
}

QList<GenericChatroomWidget*> FriendListWidget::getAllFriends()
{
    QList<GenericChatroomWidget*> friends;

    for (int i = 0; i < mainLayout->count(); ++i)
    {
        QLayout* subLayout = mainLayout->itemAt(i)->layout();

        if(!subLayout)
            continue;

        for (int j = 0; j < subLayout->count(); ++j)
        {
            GenericChatroomWidget* widget =
                reinterpret_cast<GenericChatroomWidget*>(subLayout->itemAt(j)->widget());

            if(!widget)
                continue;

            friends.append(widget);
        }
    }

    return friends;
}

void FriendListWidget::moveWidget(FriendWidget *w, Status s)
{
    QVBoxLayout* l = getFriendLayout(s);
    l->removeWidget(w); // In case the widget is already in this layout.
    Friend* g = FriendList::findFriend(static_cast<FriendWidget*>(w)->friendId);

    // Binary search.
    int min = 0, max = l->count(), mid;
    while (min < max)
    {
        mid = (max - min) / 2 + min;
        FriendWidget* w1 = static_cast<FriendWidget*>(l->itemAt(mid)->widget());
        assert(w1 != nullptr);

        Friend* f = FriendList::findFriend(w1->friendId);
        int compareValue = f->getDisplayedName().localeAwareCompare(g->getDisplayedName());
        if (compareValue > 0)
            max = mid;
        else
            min = mid + 1;
    }
    static_assert(std::is_same<decltype(w), FriendWidget*>(), "The layout must only contain FriendWidget*");
    l->insertWidget(min, w);
}

// update widget after add/delete/hide/show
void FriendListWidget::reDraw()
{
    hide();
    show();
    resize(QSize()); //lifehack
}
