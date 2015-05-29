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
#include "src/widget/friendwidget.h"
#include <QVariant>
#include <QLabel>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QLineEdit>

#include <QDragEnterEvent>
#include <QMimeData>
#include <QMenu>

#include <QDebug>

#include <cassert>

#include "friendlistlayout.h"
#include "friendlistwidget.h"

CircleWidget::CircleWidget(FriendListWidget *parent)
    : GenericChatItemWidget(parent)
{
    setStyleSheet(Style::getStylesheet(":/ui/chatroomWidgets/circleWidget.css"));

    container = new QWidget(this);
    container->setObjectName("circleWidgetContainer");
    container->setProperty("active", false);
    mainLayout = new QVBoxLayout(this);
    listLayout = new FriendListLayout();
    QHBoxLayout *layout = new QHBoxLayout();
    QVBoxLayout *midLayout = new QVBoxLayout;
    QHBoxLayout *topLayout = new QHBoxLayout;

    this->layout()->setSpacing(0);
    this->layout()->setMargin(0);
    container->setFixedHeight(55);
    setLayoutDirection(Qt::LeftToRight);

    midLayout->addStretch();

    arrowLabel = new QLabel(">", container);
    arrowLabel->setPixmap(QPixmap(":/ui/chatArea/scrollBarRightArrow.svg"));
    arrowLabel->setStyleSheet("color: white;");
    topLayout->addWidget(arrowLabel);
    topLayout->addSpacing(5);
    nameLabel = new QLabel("Circle", container);
    nameLabel->setObjectName("name");
    topLayout->addWidget(nameLabel);
    QFrame *lineFrame = new QFrame(container);
    lineFrame->setObjectName("line");
    lineFrame->setFrameShape(QFrame::HLine);

    midLayout->addLayout(topLayout);
    midLayout->addWidget(lineFrame);

    midLayout->addStretch();

    QHBoxLayout *statusLayout = new QHBoxLayout();

    onlineLabel = new QLabel("0", container);
    onlineLabel->setObjectName("status");

    statusLayout->addWidget(onlineLabel);

    topLayout->addStretch();
    topLayout->addLayout(statusLayout);

    midLayout->addStretch();

    layout->addSpacing(10);
    layout->addLayout(midLayout);
    layout->addSpacing(10);

    container->setLayout(layout);
    mainLayout->addWidget(container);

    setAcceptDrops(true);
}

void CircleWidget::addFriendWidget(FriendWidget *w, Status s)
{
    listLayout->addFriendWidget(w, s);
    //if (s == Status::Offline)
        updateOffline();
    //else
        updateOnline();
}

void CircleWidget::expand()
{
    if (expanded)
        return;
    toggle();
}

void CircleWidget::toggle()
{
    expanded = !expanded;
    if (expanded)
    {
        mainLayout->addLayout(listLayout);
        arrowLabel->setPixmap(QPixmap(":/ui/chatArea/scrollBarDownArrow.svg"));
    }
    else
    {
        mainLayout->removeItem(listLayout);
        arrowLabel->setPixmap(QPixmap(":/ui/chatArea/scrollBarRightArrow.svg"));
    }
}

void CircleWidget::searchChatrooms(const QString &searchString, bool hideOnline, bool hideOffline, bool hideGroups)
{
    listLayout->searchChatrooms(searchString, hideOnline, hideOffline, hideGroups);
}

QString CircleWidget::getName() const
{
    return nameLabel->text();
}

void CircleWidget::renameCircle()
{
    qDebug() << nameLabel->parentWidget()->layout();
    QLineEdit *lineEdit = new QLineEdit(nameLabel->text());
    lineEdit->show();
    //nameLabel->parentWidget()->layout()
    //static_cast<QBoxLayout*>(nameLabel->parentWidget()->layout())->insertWidget(nameLabel->parentWidget()->layout()->indexOf(nameLabel) - 1, lineEdit);
    //nameLabel->parentWidget()->layout()->replaceWidget(nameLabel, lineEdit);
    nameLabel->setVisible(false);
    lineEdit->selectAll();
    lineEdit->setFocus();
    connect(lineEdit, &QLineEdit::editingFinished, [this, lineEdit]()
    {
        nameLabel->setVisible(true);
        //lineEdit->parentWidget()->layout()->replaceWidget(lineEdit, nameLabel);
        nameLabel->setText(lineEdit->text());
        lineEdit->deleteLater();
    });
}

void CircleWidget::onCompactChanged(bool compact)
{

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
        while (listLayout->friendLayouts[Online]->count() != 0)
        {
            QWidget *getWidget = listLayout->friendLayouts[Online]->takeAt(0)->widget();
            assert(getWidget != nullptr);

            FriendWidget *friendWidget = dynamic_cast<FriendWidget*>(getWidget);
            friendList->moveWidget(friendWidget, FriendList::findFriend(friendWidget->friendId)->getStatus(), true);
        }
        while (listLayout->friendLayouts[Offline]->count() != 0)
        {
            QWidget *getWidget = listLayout->friendLayouts[Offline]->takeAt(0)->widget();
             assert(getWidget != nullptr);

            FriendWidget *friendWidget = dynamic_cast<FriendWidget*>(getWidget);
            friendList->moveWidget(friendWidget, FriendList::findFriend(friendWidget->friendId)->getStatus(), true);
        }

        friendList->removeCircleWidget(this);
    }
}

void CircleWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        toggle();
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
        if (!expanded)
            toggle();

        int friendId = event->mimeData()->data("friend").toInt();
        Friend *f = FriendList::findFriend(friendId);
        assert(f != nullptr);

        FriendWidget *widget = f->getFriendWidget();
        assert(widget != nullptr);

        // Update old circle after moved.
        CircleWidget *circleWidget = dynamic_cast<CircleWidget*>(widget->parent());

        addFriendWidget(widget, f->getStatus());

        if (circleWidget != nullptr)
        {
            // In case the status was changed while moving, update both.
            circleWidget->updateOffline();
            circleWidget->updateOnline();
        }

        container->setAttribute(Qt::WA_UnderMouse, false);
        Style::repolish(container);
    }
}

void CircleWidget::updateOnline()
{
    onlineLabel->setText(QString::number(listLayout->friendOnlineCount()) + QStringLiteral(" / ") + QString::number(listLayout->friendOfflineCount()));
}

void CircleWidget::updateOffline()
{
    //offlineLabel->setText(QString::number(listLayout->friendOfflineCount()));
}
