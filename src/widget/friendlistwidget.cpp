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

FriendListWidget::FriendListWidget(QWidget *parent, bool groupsOnTop)
    : QWidget(parent)
    , groupsOnTop(groupsOnTop)
{
    listLayout = new FriendListLayout();
    setLayout(listLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    circleLayout2.getLayout()->setSpacing(0);
    circleLayout2.getLayout()->setMargin(0);

    groupLayout.getLayout()->setSpacing(0);
    groupLayout.getLayout()->setMargin(0);

    listLayout->addLayout(circleLayout2.getLayout());

    onGroupchatPositionChanged(groupsOnTop);

    setAcceptDrops(true);
}

void FriendListWidget::addGroupWidget(GroupWidget *widget)
{
    groupLayout.addSortedWidget(widget);
    connect(widget, &GroupWidget::renameRequested, this, &FriendListWidget::renameGroupWidget);
}

void FriendListWidget::addFriendWidget(FriendWidget *w, Status s, int circleIndex)
{
    CircleWidget *circleWidget = nullptr;
    if (circleIndex >= 0 && circleIndex < circleLayout2.getLayout()->count())
        circleWidget = dynamic_cast<CircleWidget*>(circleLayout2.getLayout()->itemAt(circleIndex)->widget());

    if (circleWidget == nullptr)
        circleIndex = -1;

    if (circleIndex == -1)
        moveWidget(w, s, true);
    else
        circleWidget->addFriendWidget(w, s);
}

void FriendListWidget::addCircleWidget(const QString &name)
{
    CircleWidget *circleWidget = new CircleWidget(this);
    circleWidget->setName(name);
    circleLayout2.addSortedWidget(circleWidget);
    connect(this, &FriendListWidget::onCompactChanged, circleWidget, &CircleWidget::onCompactChanged);
    connect(circleWidget, &CircleWidget::renameRequested, this, &FriendListWidget::renameCircleWidget);
}

void FriendListWidget::addCircleWidget(FriendWidget *friendWidget)
{
    CircleWidget *circleWidget = new CircleWidget(this);
    circleLayout2.addSortedWidget(circleWidget);
    connect(this, &FriendListWidget::onCompactChanged, circleWidget, &CircleWidget::onCompactChanged);
    connect(circleWidget, &CircleWidget::renameRequested, this, &FriendListWidget::renameCircleWidget);
    //circleLayout->addWidget(circleWidget);
    if (friendWidget != nullptr)
    {
        circleWidget->addFriendWidget(friendWidget, FriendList::findFriend(friendWidget->friendId)->getStatus());
        circleWidget->toggle();
    }
    circleWidget->show(); // Avoid flickering.
}

void FriendListWidget::removeCircleWidget(CircleWidget *widget)
{
    circleLayout2.removeSortedWidget(widget);
    widget->deleteLater();
}

void FriendListWidget::searchChatrooms(const QString &searchString, bool hideOnline, bool hideOffline, bool hideGroups)
{
    FriendListLayout::searchLayout<GroupWidget>(searchString, groupLayout.getLayout(), hideGroups);
    listLayout->searchChatrooms(searchString, hideOnline, hideOffline);
    for (int i = 0; i != circleLayout2.getLayout()->count(); ++i)
    {
        CircleWidget *circleWidget = static_cast<CircleWidget*>(circleLayout2.getLayout()->itemAt(i)->widget());
        circleWidget->searchChatrooms(searchString, hideOnline, hideOffline);
    }
}

void FriendListWidget::renameGroupWidget(const QString &newName)
{
    assert(sender() != nullptr);

    GroupWidget* groupWidget = dynamic_cast<GroupWidget*>(sender());
    assert(groupWidget != nullptr);

    // Rename before removing so you can find it successfully.
    groupLayout.removeSortedWidget(groupWidget);
    groupWidget->setName(newName);
    groupLayout.addSortedWidget(groupWidget);
}

void FriendListWidget::renameCircleWidget(const QString &newName)
{
    assert(sender() != nullptr);

    CircleWidget* circleWidget = dynamic_cast<CircleWidget*>(sender());
    assert(circleWidget != nullptr);

    // Rename before removing so you can find it successfully.
    circleLayout2.removeSortedWidget(circleWidget);
    circleWidget->setName(newName);
    circleLayout2.addSortedWidget(circleWidget);
}

void FriendListWidget::onGroupchatPositionChanged(bool top)
{
    groupsOnTop = top;
    listLayout->removeItem(groupLayout.getLayout());
    if (top)
    {
        listLayout->insertLayout(0, groupLayout.getLayout());
    }
    else
    {
        listLayout->insertLayout(1, groupLayout.getLayout());
    }
}

void FriendListWidget::cycleContacts(GenericChatroomWidget* activeChatroomWidget, bool forward)
{
    if (activeChatroomWidget == nullptr)
        return;

    CircleWidget* circleWidget = dynamic_cast<CircleWidget*>(activeChatroomWidget->parentWidget());

    int index = -1;
    QLayout* currentLayout = nullptr;

    FriendWidget* friendWidget = dynamic_cast<FriendWidget*>(activeChatroomWidget);
    if (circleWidget != nullptr)
    {
        if (friendWidget == nullptr)
        {
            return;
        }
        if (circleWidget->cycleContacts(friendWidget, forward))
            return;

        index = circleLayout2.indexOfSortedWidget(circleWidget);
        currentLayout = circleLayout2.getLayout();
    }
    else
    {
        if (friendWidget != nullptr)
        {
            currentLayout = listLayout->getLayoutOnline();
            index = listLayout->indexOfFriendWidget(friendWidget, true);
            if (index == -1)
            {
                currentLayout = listLayout->getLayoutOffline();
                index = listLayout->indexOfFriendWidget(friendWidget, false);
            }
        }
        else
        {
            GroupWidget* groupWidget = dynamic_cast<GroupWidget*>(activeChatroomWidget);
            if (groupWidget != nullptr)
            {
                currentLayout = groupLayout.getLayout();
                index = groupLayout.indexOfSortedWidget(groupWidget);
            }
            else
            {
                return;
            };
        }
    }

    index += forward ? 1 : -1;
    for (;;)
    {
        // Bounds checking.
        if (index < 0)
        {
            currentLayout = nextLayout(currentLayout, forward);
            index = currentLayout->count() - 1;
            continue;
        }
        else if (index >= currentLayout->count())
        {
            currentLayout = nextLayout(currentLayout, forward);
            index = 0;
            continue;
        }

        // Go to the actual next index.
        if (currentLayout == listLayout->getLayoutOnline() || currentLayout == listLayout->getLayoutOffline() || currentLayout == groupLayout.getLayout())
        {
            GenericChatroomWidget* chatWidget = dynamic_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());
            if (chatWidget != nullptr)
                emit chatWidget->chatroomWidgetClicked(chatWidget);
            return;
        }
        else if (currentLayout == circleLayout2.getLayout())
        {
            circleWidget = dynamic_cast<CircleWidget*>(currentLayout->itemAt(index)->widget());
            if (circleWidget != nullptr)
            {
                if (!circleWidget->cycleContacts(forward))
                {
                    // Skip empty or finished circles.
                    index += forward ? 1 : -1;
                    continue;
                }
            }
            return;
        }
        else
        {
            return;
        }
    }
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
    vec.reserve(circleLayout2.getLayout()->count());
    for (int i = 0; i < circleLayout2.getLayout()->count(); ++i)
    {
        vec.push_back(dynamic_cast<CircleWidget*>(circleLayout2.getLayout()->itemAt(i)->widget()));
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
            circleWidget->updateStatus();
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

QLayout* FriendListWidget::nextLayout(QLayout* layout, bool forward) const
{
    if (layout == groupLayout.getLayout())
    {
        if (forward)
        {
            if (groupsOnTop)
                return listLayout->getLayoutOnline();
            return listLayout->getLayoutOffline();
        }
        else
        {
            if (groupsOnTop)
                return circleLayout2.getLayout();
            return listLayout->getLayoutOnline();
        }
    }
    else if (layout == listLayout->getLayoutOnline())
    {
        if (forward)
        {
            if (groupsOnTop)
                return listLayout->getLayoutOffline();
            return groupLayout.getLayout();
        }
        else
        {
            if (groupsOnTop)
                return groupLayout.getLayout();
            return circleLayout2.getLayout();
        }
    }
    else if (layout == listLayout->getLayoutOffline())
    {
        if (forward)
            return circleLayout2.getLayout();
        else if (groupsOnTop)
            return listLayout->getLayoutOnline();
        return groupLayout.getLayout();
    }
    else if (layout == circleLayout2.getLayout())
    {
        if (forward)
        {
            if (groupsOnTop)
                return groupLayout.getLayout();
            return listLayout->getLayoutOnline();
        }
        else
            return listLayout->getLayoutOffline();
    }
    return nullptr;
}
