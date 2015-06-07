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
#include "friendlistlayout.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/misc/settings.h"
#include "friendwidget.h"
#include "groupwidget.h"
#include "circlewidget.h"
#include <QGridLayout>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <cassert>

#include <QDebug>
#include "widget.h"

FriendListWidget::FriendListWidget(Widget* parent, bool groupsOnTop)
    : QWidget(parent)
    , groupsOnTop(groupsOnTop)
{
    listLayout = new FriendListLayout();
    setLayout(listLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    circleLayout.getLayout()->setSpacing(0);
    circleLayout.getLayout()->setMargin(0);

    groupLayout.getLayout()->setSpacing(0);
    groupLayout.getLayout()->setMargin(0);

    listLayout->addLayout(circleLayout.getLayout());

    onGroupchatPositionChanged(groupsOnTop);

    setAcceptDrops(true);
}

void FriendListWidget::addGroupWidget(GroupWidget* widget)
{
    groupLayout.addSortedWidget(widget);
    connect(widget, &GroupWidget::renameRequested, this, &FriendListWidget::renameGroupWidget);

    // Only rename group if groups are visible.
    if (Widget::getInstance()->groupsVisible())
        widget->rename();
}

void FriendListWidget::addFriendWidget(FriendWidget* w, Status s, int circleIndex)
{
    CircleWidget* circleWidget = CircleWidget::getFromID(circleIndex);
    if (circleWidget == nullptr)
        moveWidget(w, s, true);
    else
        circleWidget->addFriendWidget(w, s);
}

void FriendListWidget::addCircleWidget(int id)
{
    createCircleWidget(id);
}

void FriendListWidget::addCircleWidget(FriendWidget* friendWidget)
{
    CircleWidget* circleWidget = createCircleWidget();
    if (friendWidget != nullptr)
    {
        CircleWidget* circleOriginal = CircleWidget::getFromID(Settings::getInstance().getFriendCircleID(FriendList::findFriend(friendWidget->friendId)->getToxId()));

        circleWidget->addFriendWidget(friendWidget, FriendList::findFriend(friendWidget->friendId)->getStatus());
        circleWidget->setExpanded(true);

        Widget::getInstance()->searchCircle(circleWidget);

        if (circleOriginal != nullptr)
            Widget::getInstance()->searchCircle(circleOriginal);
    }
}

void FriendListWidget::removeCircleWidget(CircleWidget* widget)
{
    circleLayout.removeSortedWidget(widget);
    widget->deleteLater();
}

void FriendListWidget::searchChatrooms(const QString &searchString, bool hideOnline, bool hideOffline, bool hideGroups)
{
    groupLayout.search(searchString, hideGroups);
    listLayout->searchChatrooms(searchString, hideOnline, hideOffline);
    for (int i = 0; i != circleLayout.getLayout()->count(); ++i)
    {
        CircleWidget* circleWidget = static_cast<CircleWidget*>(circleLayout.getLayout()->itemAt(i)->widget());
        circleWidget->search(searchString, true, hideOnline, hideOffline);
    }
}

void FriendListWidget::renameGroupWidget(GroupWidget* groupWidget, const QString &newName)
{
    groupLayout.removeSortedWidget(groupWidget);
    groupWidget->setName(newName);
    groupLayout.addSortedWidget(groupWidget);
    reDraw(); // Prevent artifacts.
}

void FriendListWidget::renameCircleWidget(const QString &newName)
{
    assert(sender() != nullptr);

    CircleWidget* circleWidget = dynamic_cast<CircleWidget*>(sender());
    assert(circleWidget != nullptr);

    // Rename after removing so you can find it successfully.
    circleLayout.removeSortedWidget(circleWidget);
    circleWidget->setName(newName);
    circleLayout.addSortedWidget(circleWidget);
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


    int index = -1;
    QLayout* currentLayout = nullptr;

    CircleWidget* circleWidget = nullptr;
    FriendWidget* friendWidget = dynamic_cast<FriendWidget*>(activeChatroomWidget);

    if (friendWidget != nullptr)
    {
        circleWidget = CircleWidget::getFromID(Settings::getInstance().getFriendCircleID(FriendList::findFriend(friendWidget->friendId)->getToxId()));
        if (circleWidget != nullptr)
        {
            if (circleWidget->cycleContacts(friendWidget, forward))
                return;

            index = circleLayout.indexOfSortedWidget(circleWidget);
            currentLayout = circleLayout.getLayout();
        }
        else
        {
            currentLayout = listLayout->getLayoutOnline();
            index = listLayout->indexOfFriendWidget(friendWidget, true);
            if (index == -1)
            {
                currentLayout = listLayout->getLayoutOffline();
                index = listLayout->indexOfFriendWidget(friendWidget, false);
            }
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
        else if (currentLayout == circleLayout.getLayout())
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

QVector<CircleWidget*> FriendListWidget::getAllCircles()
{
    QVector<CircleWidget*> vec;
    vec.reserve(circleLayout.getLayout()->count());
    for (int i = 0; i < circleLayout.getLayout()->count(); ++i)
    {
        vec.push_back(dynamic_cast<CircleWidget*>(circleLayout.getLayout()->itemAt(i)->widget()));
    }
    return vec;
}

void FriendListWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("friend"))
        event->acceptProposedAction();
}

void FriendListWidget::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasFormat("friend"))
    {
        int friendId = event->mimeData()->data("friend").toInt();
        Friend* f = FriendList::findFriend(friendId);
        assert(f != nullptr);

        FriendWidget* widget = f->getFriendWidget();
        assert(widget != nullptr);

        // Update old circle after moved.
        CircleWidget* circleWidget = CircleWidget::getFromID(Settings::getInstance().getFriendCircleID(f->getToxId()));

        listLayout->addFriendWidget(widget, f->getStatus());

        if (circleWidget != nullptr)
        {
            // In case the status was changed while moving, update both.
            circleWidget->updateStatus();
        }
    }
}

void FriendListWidget::moveWidget(FriendWidget* w, Status s, bool add)
{
    int circleId = Settings::getInstance().getFriendCircleID(FriendList::findFriend(w->friendId)->getToxId());
    CircleWidget* circleWidget = CircleWidget::getFromID(circleId);

    if (circleWidget == nullptr || add)
    {
        if (circleId != -1)
            Settings::getInstance().setFriendCircleID(FriendList::findFriend(w->friendId)->getToxId(), -1);
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

CircleWidget* FriendListWidget::createCircleWidget(int id)
{
    CircleWidget* circleWidget = new CircleWidget(this, id);
    circleLayout.addSortedWidget(circleWidget);
    connect(this, &FriendListWidget::onCompactChanged, circleWidget, &CircleWidget::onCompactChanged);
    connect(circleWidget, &CircleWidget::renameRequested, this, &FriendListWidget::renameCircleWidget);
    circleWidget->show(); // Avoid flickering.
    return circleWidget;
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
                return circleLayout.getLayout();
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
            return circleLayout.getLayout();
        }
    }
    else if (layout == listLayout->getLayoutOffline())
    {
        if (forward)
            return circleLayout.getLayout();
        else if (groupsOnTop)
            return listLayout->getLayoutOnline();
        return groupLayout.getLayout();
    }
    else if (layout == circleLayout.getLayout())
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
