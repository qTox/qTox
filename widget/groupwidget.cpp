/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

#include "groupwidget.h"
#include "grouplist.h"
#include "group.h"
#include <QPalette>
#include <QMenu>
#include <QContextMenuEvent>

GroupWidget::GroupWidget(int GroupId, QString Name)
    : groupId{GroupId}
{
    this->setMouseTracking(true);
    this->setAutoFillBackground(true);
    this->setLayout(&layout);
    this->setFixedWidth(225);
    this->setFixedHeight(55);
    layout.setSpacing(0);
    layout.setMargin(0);
    textLayout.setSpacing(0);
    textLayout.setMargin(0);

    avatar.setPixmap(QPixmap(":img/group.png"));
    statusPic.setPixmap(QPixmap(":img/status/dot_online.png"));
    name.setText(Name);
    QFont small;
    small.setPixelSize(10);
    nusers.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::gray);
    nusers.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::white);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, QColor(65,65,65,255));
    this->setPalette(pal3);
    Group* g = GroupList::findGroup(groupId);
    if (g)
        nusers.setText(GroupWidget::tr("%1 users in chat").arg(g->peers.size()));
    else
        nusers.setText(GroupWidget::tr("0 users in chat"));

    textLayout.addStretch();
    textLayout.addWidget(&name);
    textLayout.addWidget(&nusers);
    textLayout.addStretch();

    layout.addSpacing(20);
    layout.addWidget(&avatar);
    layout.addSpacing(5);
    layout.addLayout(&textLayout);
    layout.addStretch();
    layout.addSpacing(5);
    layout.addWidget(&statusPic);
    layout.addSpacing(5);

    isActiveWidget = 0;
}

void GroupWidget::setNewFixedWidth(int newWidth)
{
    this->setFixedWidth(newWidth);
}

void GroupWidget::mouseReleaseEvent (QMouseEvent*)
{
    emit groupWidgetClicked(this);
}

void GroupWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    QMenu menu;
    QAction* quitGroup = menu.addAction(tr("Quit group","Menu to quit a groupchat"));

    QAction* selectedItem = menu.exec(pos);
    if (selectedItem == quitGroup)
    {
        hide();
        show(); //Toggle visibility to work around bug of repaintEvent() not being fired on parent widget when this is hidden
        hide();
        emit removeGroup(groupId);
        return;
    }
}

void GroupWidget::mousePressEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) == Qt::LeftButton)
    {
        if (isActiveWidget)
        {
            QPalette pal;
            pal.setColor(QPalette::Background, QColor(250,250,250,255));
            this->setPalette(pal);
        }
        else
        {
            QPalette pal;
            pal.setColor(QPalette::Background, QColor(85,85,85,255));
            this->setPalette(pal);
        }
    }
}

void GroupWidget::enterEvent(QEvent*)
{
    if (isActiveWidget != 1)
    {
        QPalette pal;
        pal.setColor(QPalette::Background, QColor(75,75,75,255));
        lastColor = this->palette().background().color();
        this->setPalette(pal);
    }
}

void GroupWidget::leaveEvent(QEvent*)
{
    if (isActiveWidget != 1)
    {
        QPalette pal;
        pal.setColor(QPalette::Background, lastColor);
        this->setPalette(pal);
    }
}

void GroupWidget::onUserListChanged()
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
        nusers.setText(tr("%1 users in chat").arg(g->nPeers));
    else
        nusers.setText(tr("0 users in chat"));
}

void GroupWidget::setAsActiveChatroom()
{
    isActiveWidget = 1;

    QFont small;
    small.setPixelSize(10);
    nusers.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::darkGray);
    nusers.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::black);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, Qt::white);
    this->setPalette(pal3);
    avatar.setPixmap(QPixmap(":img/group_dark.png"));
}

void GroupWidget::setAsInactiveChatroom()
{
    isActiveWidget = 0;

    QFont small;
    small.setPixelSize(10);
    nusers.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::gray);
    nusers.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::white);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, QColor(65,65,65,255));
    this->setPalette(pal3);
    avatar.setPixmap(QPixmap(":img/group.png"));
}
