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

#include "circlewidget.hpp"
#include "src/misc/style.h"
#include "src/misc/settings.h"
#include "src/friendlist.h"
#include "src/friend.h"
#include "src/widget/friendwidget.h"
#include <QVariant>
#include <QLabel>
#include <QBoxLayout>

#include <QDragEnterEvent>
#include <QMimeData>

#include <QDebug>

#include <cassert>

CircleWidget::CircleWidget(QWidget *parent)
    : QFrame(parent)
{
    setProperty("compact", Settings::getInstance().getCompactLayout());

    setProperty("active", false);

    setStyleSheet(Style::getStylesheet(":/ui/chatroomWidgets/circleWidget.css"));

    QWidget *container = new QWidget(this);
    container->setObjectName("circleWidgetContainer");
    container->setProperty("active", false);
    mainLayout = new QVBoxLayout(this);
    groupLayout = new QVBoxLayout(this);
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
    QLabel *nameLabel = new QLabel("Circle", container);
    nameLabel->setObjectName("name");
    topLayout->addWidget(nameLabel);
    QFrame *lineFrame = new QFrame(container);
    lineFrame->setObjectName("line");
    lineFrame->setFrameShape(QFrame::HLine);
    topLayout->addSpacing(5);
    topLayout->addWidget(lineFrame, 1);

    midLayout->addLayout(topLayout);

    midLayout->addStretch();

    QHBoxLayout *statusLayout = new QHBoxLayout();

    QLabel *onlineIconLabel = new QLabel(container);
    onlineIconLabel->setAlignment(Qt::AlignCenter);
    onlineIconLabel->setPixmap(QPixmap(":img/status/dot_online.svg"));
    QLabel *onlineLabel = new QLabel("0", container);
    onlineLabel->setObjectName("status");

    QLabel *awayIconLabel = new QLabel(container);
    awayIconLabel->setAlignment(Qt::AlignCenter);
    awayIconLabel->setPixmap(QPixmap(":img/status/dot_away.svg"));
    QLabel *awayLabel = new QLabel("0", container);
    awayLabel->setObjectName("status");

    QLabel *offlineIconLabel = new QLabel(container);
    offlineIconLabel->setAlignment(Qt::AlignCenter);
    offlineIconLabel->setPixmap(QPixmap(":img/status/dot_offline.svg"));
    QLabel *offlineLabel = new QLabel("0", container);
    offlineLabel->setObjectName("status");

    statusLayout->addWidget(onlineIconLabel);
    statusLayout->addSpacing(5);
    statusLayout->addWidget(onlineLabel);
    statusLayout->addSpacing(10);
    statusLayout->addWidget(awayIconLabel);
    statusLayout->addSpacing(5);
    statusLayout->addWidget(awayLabel);
    statusLayout->addSpacing(10);
    statusLayout->addWidget(offlineIconLabel);
    statusLayout->addSpacing(5);
    statusLayout->addWidget(offlineLabel);
    statusLayout->addStretch();

    midLayout->addLayout(statusLayout);

    midLayout->addStretch();

    layout->addSpacing(10);
    layout->addLayout(midLayout);
    layout->addSpacing(10);

    container->setLayout(layout);
    mainLayout->addWidget(container);

    setAcceptDrops(true);
}

bool CircleWidget::isCompact() const
{
    return compact;
}

void CircleWidget::setCompact(bool compact)
{
    this->compact = compact;
    Style::repolish(this);
}

void CircleWidget::toggle()
{
    visible = !visible;
    if (visible)
    {
        mainLayout->addLayout(groupLayout);
        arrowLabel->setPixmap(QPixmap(":/ui/chatArea/scrollBarDownArrow.svg"));
    }
    else
    {
        mainLayout->removeItem(groupLayout);
        arrowLabel->setPixmap(QPixmap(":/ui/chatArea/scrollBarRightArrow.svg"));
    }
}

void CircleWidget::mousePressEvent(QMouseEvent *event)
{
    toggle();
}

void CircleWidget::dragEnterEvent(QDragEnterEvent *event)
{
  qDebug() << event->mimeData();
    if (event->mimeData()->hasFormat("friend"))
        event->acceptProposedAction();
}

void CircleWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("friend"))
    {
        int friendId = event->mimeData()->data("friend").toInt();
        Friend *f = FriendList::findFriend(friendId);
        assert(f != nullptr);

        FriendWidget *widget = f->getFriendWidget();
        assert(widget != nullptr);

        groupLayout->addWidget(widget);
    }
}
