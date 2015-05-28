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
#include "circlewidget.h"
#include "friendlistlayout.h"
#include <cassert>

FriendListWidget::FriendListWidget(QWidget *parent, bool groupsOnTop) :
    QWidget(parent)
{
    listLayout = new FriendListLayout(this, groupsOnTop);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    circleLayout = new QVBoxLayout();
    circleLayout->setSpacing(0);
    circleLayout->setMargin(0);

    listLayout->addLayout(circleLayout);
}

void FriendListWidget::addGroupWidget(GroupWidget *widget)
{
    listLayout->groupLayout->addWidget(widget);
}

void FriendListWidget::hideGroups(QString searchString, bool hideAll)
{
    QVBoxLayout* groups = listLayout->groupLayout;
    int groupCount = groups->count(), index;

    for (index = 0; index<groupCount; index++)
    {
        GroupWidget* groupWidget = static_cast<GroupWidget*>(groups->itemAt(index)->widget());
        QString groupName = groupWidget->getName();

        groupWidget->setVisible(groupName.contains(searchString, Qt::CaseInsensitive) && !hideAll);
    }
}

void FriendListWidget::addCircleWidget(CircleWidget *widget)
{
    circleLayout->addWidget(widget);
}

void FriendListWidget::searchChatrooms(const QString &searchString, bool hideOnline, bool hideOffline, bool hideGroups)
{
    listLayout->searchChatrooms(searchString, hideOnline, hideOffline, hideGroups);
    for (int i = 0; i != circleLayout->count(); ++i)
    {
        CircleWidget *circleWidget = static_cast<CircleWidget*>(circleLayout->itemAt(i)->widget());
        circleWidget->searchChatrooms(searchString, hideOnline, hideOffline, hideGroups);
    }
}

void FriendListWidget::hideFriends(QString searchString, Status status, bool hideAll)
{
    QVBoxLayout* friends = getFriendLayout(status);
    int friendCount = friends->count(), index;

    for (index = 0; index<friendCount; index++)
    {
        FriendWidget* friendWidget = static_cast<FriendWidget*>(friends->itemAt(index)->widget());
        QString friendName = friendWidget->getName();

        friendWidget->setVisible(friendName.contains(searchString, Qt::CaseInsensitive) && !hideAll);
    }
}

QVBoxLayout* FriendListWidget::getFriendLayout(Status s)
{
    return s == Status::Offline ? listLayout->friendLayouts[Offline] : listLayout->friendLayouts[Online];
}

void FriendListWidget::onGroupchatPositionChanged(bool top)
{
    listLayout->removeItem(circleLayout);
    listLayout->removeItem(listLayout->groupLayout);
    listLayout->removeItem(listLayout->friendLayouts[Online]);
    if (top)
    {
        listLayout->addLayout(listLayout->groupLayout);
        listLayout->addLayout(listLayout->friendLayouts[Online]);
    }
    else
    {
        listLayout->addLayout(listLayout->friendLayouts[Online]);
        listLayout->addLayout(listLayout->groupLayout);
    }
    listLayout->addLayout(circleLayout);
}

QList<GenericChatroomWidget*> FriendListWidget::getAllFriends()
{
    QList<GenericChatroomWidget*> friends;

    for (int i = 0; i < listLayout->count(); ++i)
    {
        QLayout* subLayout = listLayout->itemAt(i)->layout();

        if(!subLayout)
            continue;

        for (int j = 0; j < subLayout->count(); ++j)
        {
            GenericChatroomWidget* widget =
                dynamic_cast<GenericChatroomWidget*>(subLayout->itemAt(j)->widget());

            if(!widget)
                continue;

            friends.append(widget);
        }
    }

    return friends;
}

void FriendListWidget::moveWidget(FriendWidget *w, Status s, bool add)
{
    CircleWidget *circleWidget = dynamic_cast<CircleWidget*>(w->parent());

    if (circleWidget == nullptr || add)
    {
        listLayout->addFriendWidget(w, s);
        return;
    }

    circleWidget->addFriendWidget(w, s);
}

// update widget after add/delete/hide/show
void FriendListWidget::reDraw()
{
    hide();
    show();
    resize(QSize()); //lifehack
}
