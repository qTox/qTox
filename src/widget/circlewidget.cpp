/*
    Copyright © 2015 by The qTox Project

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

#include "circlewidget.h"
#include "friendwidget.h"
#include "friendlistwidget.h"
#include "tool/croppinglabel.h"
#include "src/persistence/settings.h"
#include "src/friendlist.h"
#include "src/friend.h"
#include "src/widget/contentdialog.h"
#include "widget.h"
#include <QVariant>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMenu>
#include <cassert>

QHash<int, CircleWidget*> CircleWidget::circleList;

CircleWidget::CircleWidget(FriendListWidget* parent, int id)
    : CategoryWidget(parent)
    , id(id)
{
    const Settings& s = Settings::getInstance();

    setName(s.getCircleName(id), false);
    circleList[id] = this;

    connect(nameLabel, &CroppingLabel::editFinished, [this](const QString &newName)
    {
        if (!newName.isEmpty())
            emit renameRequested(this, newName);
    });

    connect(nameLabel, &CroppingLabel::editRemoved, [this]()
    {
        if (Settings::getInstance().getCompactLayout())
            nameLabel->minimizeMaximumWidth();
    });

    setExpanded(s.getCircleExpanded(id), false);
    updateStatus();
}

CircleWidget::~CircleWidget()
{
    if (circleList[id] == this)
        circleList.remove(id);
}

void CircleWidget::editName()
{
    CategoryWidget::editName();
}

CircleWidget* CircleWidget::getFromID(int id)
{
    auto circleIt = circleList.find(id);

    if (circleIt != circleList.end())
        return circleIt.value();

    return nullptr;
}

void CircleWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;
    QAction* renameAction = menu.addAction(tr("Rename circle", "Menu for renaming a circle"));
    QAction* removeAction = menu.addAction(tr("Remove circle", "Menu for removing a circle"));
    QAction* openAction = nullptr;

    if (friendOfflineLayout()->count() + friendOnlineLayout()->count() > 0)
        openAction = menu.addAction(tr("Open all in new window"));

    QAction* selectedItem = menu.exec(mapToGlobal(event->pos()));

    if (selectedItem)
    {
        if (selectedItem == renameAction)
        {
            editName();
        }
        else if (selectedItem == removeAction)
        {
            FriendListWidget* friendList = static_cast<FriendListWidget*>(parentWidget());
            moveFriendWidgets(friendList);

            friendList->removeCircleWidget(this);

            int replacedCircle = Settings::getInstance().removeCircle(id);

            auto circleReplace = circleList.find(replacedCircle);
            if (circleReplace != circleList.end())
                circleReplace.value()->updateID(id);
            else
                assert(true); // This should never happen.

            circleList.remove(replacedCircle);
        }
        else if (selectedItem == openAction)
        {
            ContentDialog* dialog = Widget::getInstance()->createContentDialog();

            for (int i = 0; i < friendOnlineLayout()->count(); ++i)
            {
                FriendWidget* friendWidget = qobject_cast<FriendWidget*>(friendOnlineLayout()->itemAt(i)->widget());

                if (friendWidget != nullptr)
                    emit friendWidget->chatroomWidgetClicked(friendWidget, false);
            }
            for (int i = 0; i < friendOfflineLayout()->count(); ++i)
            {
                FriendWidget* friendWidget = qobject_cast<FriendWidget*>(friendOfflineLayout()->itemAt(i)->widget());

                if (friendWidget != nullptr)
                    emit friendWidget->chatroomWidgetClicked(friendWidget, false);
            }

            dialog->show();
            dialog->ensureSplitterVisible();
        }
    }

    setContainerAttribute(Qt::WA_UnderMouse, false);
}

void CircleWidget::dragEnterEvent(QDragEnterEvent* event)
{
    ToxId toxId(event->mimeData()->text());
    Friend *f = FriendList::findFriend(toxId);
    if (f != nullptr)
        event->acceptProposedAction();

    setContainerAttribute(Qt::WA_UnderMouse, true); // Simulate hover.
}

void CircleWidget::dragLeaveEvent(QDragLeaveEvent* )
{
    setContainerAttribute(Qt::WA_UnderMouse, false);
}

void CircleWidget::dropEvent(QDropEvent* event)
{
    // Check, if element dropped from qTox
    QObject *o = event->source();
    FriendWidget *widget = qobject_cast<FriendWidget*>(o);
    if (!widget)
        return;

    // Check, if a friend with this toxId is contained in our friend list
    ToxId toxId(event->mimeData()->text());
    Friend *f = FriendList::findFriend(toxId);
    if (!f)
        return;

    setExpanded(true, false);
    addFriendWidget(widget, f->getStatus());
    Settings::getInstance().savePersonal();

    // Update old circle after moved.
    int circleId = Settings::getInstance().getFriendCircleID(f->getToxId());
    CircleWidget* circleWidget = getFromID(circleId);
    if (circleWidget != nullptr)
    {
        circleWidget->updateStatus();
        Widget::getInstance()->searchCircle(circleWidget);
    }

    setContainerAttribute(Qt::WA_UnderMouse, false);
}

void CircleWidget::onSetName()
{
    Settings::getInstance().setCircleName(id, getName());
}

void CircleWidget::onExpand()
{
    Settings::getInstance().setCircleExpanded(id, isExpanded());
    Settings::getInstance().savePersonal();
}

void CircleWidget::onAddFriendWidget(FriendWidget* w)
{
    Settings::getInstance().setFriendCircleID(FriendList::findFriend(w->friendId)->getToxId(), id);
}

void CircleWidget::updateID(int index)
{
    // For when a circle gets destroyed, another takes its id.
    // This function updates all friends widgets for this new id.

    if (id == index)
        return;

    id = index;
    circleList[id] = this;

    for (int i = 0; i < friendOnlineLayout()->count(); ++i)
    {
        FriendWidget* friendWidget = qobject_cast<FriendWidget*>(friendOnlineLayout()->itemAt(i)->widget());

        if (friendWidget != nullptr)
            Settings::getInstance().setFriendCircleID(FriendList::findFriend(friendWidget->friendId)->getToxId(), id);
    }
    for (int i = 0; i < friendOfflineLayout()->count(); ++i)
    {
        FriendWidget* friendWidget = qobject_cast<FriendWidget*>(friendOfflineLayout()->itemAt(i)->widget());

        if (friendWidget != nullptr)
            Settings::getInstance().setFriendCircleID(FriendList::findFriend(friendWidget->friendId)->getToxId(), id);
    }
}
