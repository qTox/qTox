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

#include "circlewidget.h"
#include "src/misc/style.h"
#include "src/misc/settings.h"
#include "src/friendlist.h"
#include "src/friend.h"
#include "friendwidget.h"
#include "friendlistlayout.h"
#include "friendlistwidget.h"
#include "croppinglabel.h"
#include "widget.h"
#include <QVariant>
#include <QLabel>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMenu>
#include <cassert>

void maxCropLabel(CroppingLabel* label)
{
    QFontMetrics metrics = label->fontMetrics();
    // Text width + padding. Without padding, we'll have elipses.
    label->setMaximumWidth(metrics.width(label->fullText()) + metrics.width("..."));
}

QHash<int, CircleWidget*> CircleWidget::circleList;

CircleWidget::CircleWidget(FriendListWidget *parent, int id_)
    : GenericChatItemWidget(parent)
    , id(id_)
{
    setStyleSheet(Style::getStylesheet(":/ui/chatroomWidgets/circleWidget.css"));

    container = new QWidget(this);
    container->setObjectName("circleWidgetContainer");
    container->setLayoutDirection(Qt::LeftToRight);

    // status text
    statusLabel = new QLabel(this);
    statusLabel->setObjectName("status");
    statusLabel->setTextFormat(Qt::PlainText);

    statusPic.setPixmap(QPixmap(":/ui/chatArea/scrollBarRightArrow.svg"));

    fullLayout = new QVBoxLayout(this);
    fullLayout->setSpacing(0);
    fullLayout->setMargin(0);
    fullLayout->addWidget(container);

    lineFrame = new QFrame(container);
    lineFrame->setObjectName("line");
    lineFrame->setFrameShape(QFrame::HLine);
    lineFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    lineFrame->resize(0, 0);

    listLayout = new FriendListLayout();
    listWidget = new QWidget(this);
    listWidget->setLayout(listLayout);
    fullLayout->addWidget(listWidget);

    setAcceptDrops(true);

    onCompactChanged(isCompact());

    if (id != -1)
    {
        setName(Settings::getInstance().getCircleName(id));
    }

    connect(nameLabel, &CroppingLabel::editFinished, [this](const QString &newName)
    {
        if (!newName.isEmpty())
        {
            emit renameRequested(newName);
        }
    });

    bool isNew = false;
    auto circleIt = circleList.find(id);
    if (circleIt == circleList.end())
    {
        if (id == -1)
        {
            isNew = true;
            id = Settings::getInstance().addCircle();
            nameLabel->setText(tr("Circle #%1").arg(id + 1));
            Settings::getInstance().setCircleName(id, nameLabel->fullText());
        }
    }
    circleList[id] = this;

    if (isNew)
        renameCircle();

    setExpanded(Settings::getInstance().getCircleExpanded(id));

    updateStatus();
}

void CircleWidget::addFriendWidget(FriendWidget *w, Status s)
{
    listLayout->addFriendWidget(w, s);
    updateStatus();
    Settings::getInstance().setFriendCircleID(FriendList::findFriend(w->friendId)->getToxId(), id);
}

void CircleWidget::setExpanded(bool isExpanded)
{
    expanded = isExpanded;
    listWidget->setVisible(isExpanded);
    if (isExpanded)
    {
        statusPic.setPixmap(QPixmap(":/ui/chatArea/scrollBarDownArrow.svg"));
    }
    else
    {
        statusPic.setPixmap(QPixmap(":/ui/chatArea/scrollBarRightArrow.svg"));
    }

    Settings::getInstance().setCircleExpanded(id, isExpanded);
}

void CircleWidget::search(const QString &searchString, bool updateAll, bool hideOnline, bool hideOffline)
{
    if (updateAll)
    {
        listLayout->searchChatrooms(searchString, hideOnline, hideOffline);
    }
    bool inCategory = searchString.isEmpty() && !(hideOnline && hideOffline);
    setVisible(inCategory || listLayout->hasChatrooms());
}

void CircleWidget::setName(const QString &name)
{
    nameLabel->setText(name);
    Settings::getInstance().setCircleName(id, name);
    if (isCompact())
        maxCropLabel(nameLabel);
}

void CircleWidget::renameCircle()
{
    nameLabel->editStart();
    nameLabel->setMaximumWidth(QWIDGETSIZE_MAX);
}

void emitChatroomWidget(QLayout* layout, int index)
{
    GenericChatroomWidget* chatWidget = dynamic_cast<GenericChatroomWidget*>(layout->itemAt(index)->widget());
    if (chatWidget != nullptr)
        emit chatWidget->chatroomWidgetClicked(chatWidget);
}

bool CircleWidget::cycleContacts(bool forward)
{
    if (listLayout->friendTotalCount() == 0)
    {
        return false;
    }
    if (forward)
    {
        if (listLayout->getLayoutOnline()->count() != 0)
        {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOnline(), 0);
            return true;
        }
        else if (listLayout->getLayoutOffline()->count() != 0)
        {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOffline(), 0);
            return true;
        }
    }
    else
    {
        if (listLayout->getLayoutOffline()->count() != 0)
        {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOffline(), listLayout->getLayoutOffline()->count() - 1);
            return true;
        }
        else if (listLayout->getLayoutOnline()->count() != 0)
        {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOnline(), listLayout->getLayoutOnline()->count() - 1);
            return true;
        }
    }
    return false;
}

bool CircleWidget::cycleContacts(FriendWidget *activeChatroomWidget, bool forward)
{
    int index = -1;
    QLayout* currentLayout = nullptr;

    FriendWidget* friendWidget = dynamic_cast<FriendWidget*>(activeChatroomWidget);
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
        return false;

    index += forward ? 1 : -1;
    for (;;)
    {
        // Bounds checking.
        if (index < 0)
        {
            if (currentLayout == listLayout->getLayoutOffline())
                currentLayout = listLayout->getLayoutOnline();
            else
                return false;

            index = currentLayout->count() - 1;
            continue;
        }
        else if (index >= currentLayout->count())
        {
            if (currentLayout == listLayout->getLayoutOnline())
                currentLayout = listLayout->getLayoutOffline();
            else
                return false;

            index = 0;
            continue;
        }

        GenericChatroomWidget* chatWidget = dynamic_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());
        if (chatWidget != nullptr)
            emit chatWidget->chatroomWidgetClicked(chatWidget);
        return true;
    }

    return false;
}

CircleWidget* CircleWidget::getFromID(int id)
{
    auto circleIt = circleList.find(id);
    if (circleIt != circleList.end())
        return circleIt.value();
    return nullptr;
}

void CircleWidget::onCompactChanged(bool _compact)
{
    delete topLayout;
    delete mainLayout;

    topLayout = new QHBoxLayout;
    topLayout->setSpacing(0);
    topLayout->setMargin(0);

    setProperty("compact", _compact);

    if (property("compact").toBool())
    {
        maxCropLabel(nameLabel);

        mainLayout = nullptr;

        container->setFixedHeight(25);
        container->setLayout(topLayout);

        topLayout->addSpacing(18);
        topLayout->addWidget(&statusPic);
        topLayout->addSpacing(5);
        topLayout->addWidget(nameLabel, 100);
        topLayout->addWidget(lineFrame, 1);
        topLayout->addSpacing(5);
        topLayout->addWidget(statusLabel);
        topLayout->addSpacing(5);
        topLayout->activate();
    }
    else
    {
        nameLabel->setMaximumWidth(QWIDGETSIZE_MAX);

        mainLayout = new QVBoxLayout();
        mainLayout->setSpacing(0);
        mainLayout->setContentsMargins(20, 0, 20, 0);

        container->setFixedHeight(55);
        container->setLayout(mainLayout);

        topLayout->addWidget(&statusPic);
        topLayout->addSpacing(10);
        topLayout->addWidget(nameLabel, 1);
        topLayout->addSpacing(5);
        topLayout->addWidget(statusLabel);
        topLayout->activate();

        mainLayout->addStretch();
        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(lineFrame);
        mainLayout->addStretch();
        mainLayout->activate();
    }

    Style::repolish(this);
}

void CircleWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    QAction *renameAction = menu.addAction(tr("Rename circle", "Menu for renaming a circle"));
    QAction *removeAction = menu.addAction(tr("Remove circle", "Menu for removing a circle"));

    QAction *selectedItem = menu.exec(mapToGlobal(event->pos()));
    if (selectedItem == renameAction)
        renameCircle();
    else if (selectedItem == removeAction)
    {
        FriendListWidget *friendList = static_cast<FriendListWidget*>(parentWidget());
        listLayout->moveFriendWidgets(friendList);

        friendList->removeCircleWidget(this);

        circleList.remove(id);
        int replacedCircle = Settings::getInstance().removeCircle(id);

        auto circleReplace = circleList.find(replacedCircle);
        if (circleReplace != circleList.end())
            circleReplace.value()->updateID(id);
    }
}

void CircleWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        setExpanded(!expanded);
}

void CircleWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("friend"))
        event->acceptProposedAction();
    container->setAttribute(Qt::WA_UnderMouse, true); // Simulate hover.
    Style::repolish(container);
}

void CircleWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    container->setAttribute(Qt::WA_UnderMouse, false);
    Style::repolish(container);
}

void CircleWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("friend"))
    {
        setExpanded(true);

        int friendId = event->mimeData()->data("friend").toInt();
        Friend *f = FriendList::findFriend(friendId);
        assert(f != nullptr);

        FriendWidget *widget = f->getFriendWidget();
        assert(widget != nullptr);

        // Update old circle after moved.
        CircleWidget *circleWidget = getFromID(Settings::getInstance().getFriendCircleID(f->getToxId()));

        addFriendWidget(widget, f->getStatus());

        if (circleWidget != nullptr)
        {
            circleWidget->updateStatus();
            Widget::getInstance()->searchCircle(circleWidget);
        }

        container->setAttribute(Qt::WA_UnderMouse, false);
        Style::repolish(container);
    }
}

void CircleWidget::updateStatus()
{
    statusLabel->setText(QString::number(listLayout->friendOnlineCount()) + QStringLiteral(" / ") + QString::number(listLayout->friendTotalCount()));
}

void CircleWidget::updateID(int index)
{
    // For when a circle gets destroyed, another takes its id.
    // This function updates all friends widgets for this new id.
    id = index;
    circleList[id] = this;

    for (int i = 0; i < listLayout->getLayoutOnline()->count(); ++i)
    {
        FriendWidget* friendWidget = dynamic_cast<FriendWidget*>(listLayout->getLayoutOnline()->itemAt(i));
        if (friendWidget != nullptr)
        {
            Settings::getInstance().setFriendCircleID(FriendList::findFriend(friendWidget->friendId)->getToxId(), id);
        }
    }
    for (int i = 0; i < listLayout->getLayoutOffline()->count(); ++i)
    {
        FriendWidget* friendWidget = dynamic_cast<FriendWidget*>(listLayout->getLayoutOffline()->itemAt(i));
        if (friendWidget != nullptr)
        {
            Settings::getInstance().setFriendCircleID(FriendList::findFriend(friendWidget->friendId)->getToxId(), id);
        }
    }
}
