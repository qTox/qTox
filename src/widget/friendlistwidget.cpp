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
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
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
    listLayout = new FriendListLayout(groupsOnTop);
    setLayout(listLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    circleLayout = new QVBoxLayout();
    circleLayout->setSpacing(0);
    circleLayout->setMargin(0);

    listLayout->addLayout(circleLayout);

    setAcceptDrops(true);
}

void FriendListWidget::addGroupWidget(GroupWidget *widget)
{
    listLayout->groupLayout->addWidget(widget);
}

void FriendListWidget::addCircleWidget(FriendWidget *friendWidget)
{
    CircleWidget *circleWidget = new CircleWidget(this);
    circleLayout->addWidget(circleWidget);
    if (friendWidget != nullptr)
    {
        circleWidget->addFriendWidget(friendWidget, FriendList::findFriend(friendWidget->friendId)->getStatus());
        circleWidget->toggle();
    }
    circleWidget->show(); // Avoid flickering.
}

void FriendListWidget::removeCircleWidget(CircleWidget *widget)
{
    //setUpdatesEnabled(false);
    //widget->setVisible(false);
    circleLayout->removeWidget(widget);
    widget->deleteLater();
    //setUpdatesEnabled(true);
    //widget->deleteLater();
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

QVector<CircleWidget*> FriendListWidget::getAllCircles()
{
    QVector<CircleWidget*> vec;
    vec.reserve(circleLayout->count());
    for (int i = 0; i < circleLayout->count(); ++i)
    {
        vec.push_back(dynamic_cast<CircleWidget*>(circleLayout->itemAt(i)->widget()));
    }
    return vec;
}

void FriendListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("friend"))
        event->acceptProposedAction();
}

void FriendListWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("friend"))
    {
        int friendId = event->mimeData()->data("friend").toInt();
        Friend *f = FriendList::findFriend(friendId);
        assert(f != nullptr);

        FriendWidget *widget = f->getFriendWidget();
        assert(widget != nullptr);

        // Update old circle after moved.
        CircleWidget *circleWidget = dynamic_cast<CircleWidget*>(widget->parent());

        listLayout->addFriendWidget(widget, f->getStatus());

        if (circleWidget != nullptr)
        {
            // In case the status was changed while moving, update both.
            circleWidget->updateOffline();
            circleWidget->updateOnline();
        }
    }
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
