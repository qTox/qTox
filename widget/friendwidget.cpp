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

#include "friendwidget.h"
#include "group.h"
#include "grouplist.h"
#include "groupwidget.h"
#include "friendlist.h"
#include "friend.h"
#include "core.h"
#include "widget/form/chatform.h"
#include "widget/maskablepixmapwidget.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QBitmap>

FriendWidget::FriendWidget(int FriendId, QString id)
    : friendId(FriendId), isDefaultAvatar{true}
{
    setMouseTracking(true);
    setAutoFillBackground(true);
    setFixedHeight(55);
    setLayout(&layout);
    layout.setSpacing(0);
    layout.setMargin(0);
    layout.setStretchFactor(this, 100);
    textLayout.setSpacing(0);
    textLayout.setMargin(0);
    setLayoutDirection(Qt::LeftToRight); // parent might have set Qt::RightToLeft

    avatar = new MaskablePixmapWidget(this, QSize(40,40), ":/img/avatar_mask.png");
    avatar->setPixmap(QPixmap(":img/contact_dark.png"));
    name.setText(id);
    //statusPic.setAlignment(Qt::AlignHCenter);
    statusPic.setPixmap(QPixmap(":img/status/dot_away.png"));
    QFont small;
    small.setPixelSize(10);
    statusMessage.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::gray);
    statusMessage.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::white);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, QColor(65,65,65,255));
    this->setPalette(pal3);

    textLayout.addStretch();
    textLayout.addWidget(&name);
    textLayout.addWidget(&statusMessage);
    textLayout.addStretch();

    layout.addSpacing(20);
    layout.addWidget(avatar);
    layout.addSpacing(5);
    layout.addLayout(&textLayout);
    layout.addSpacing(5);
    layout.addWidget(&statusPic);
    layout.addSpacing(5);

    isActiveWidget = 0;

    layout.invalidate();
    layout.update();
    layout.activate();
    updateGeometry();
}

void FriendWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    QMenu menu;
    QAction* copyId = menu.addAction(tr("Copy friend ID","Menu to copy the Tox ID of that friend"));
    QMenu* inviteMenu = menu.addMenu(tr("Invite in group","Menu to invite a friend in a groupchat"));
    QMap<QAction*, Group*> groupActions;
    for (Group* group : GroupList::groupList)
    {
        QAction* groupAction = inviteMenu->addAction(group->widget->name.text());
        groupActions[groupAction] =  group;
    }
    if (groupActions.isEmpty())
        inviteMenu->setEnabled(false);
    menu.addSeparator();
    QAction* removeFriendAction = menu.addAction(tr("Remove friend", "Menu to remove the friend from our friendlist"));

    QAction* selectedItem = menu.exec(pos);
    if (selectedItem)
    {
        if (selectedItem == copyId)
        {
            emit copyFriendIdToClipboard(friendId);
            return;
        }
        else if (selectedItem == removeFriendAction)
        {
            hide();
            show(); //Toggle visibility to work around bug of repaintEvent() not being fired on parent widget when this is hidden
            hide();
            emit removeFriend(friendId);
            return;
        }
        else if (groupActions.contains(selectedItem))
        {
            Group* group = groupActions[selectedItem];
            Core::getInstance()->groupInviteFriend(friendId, group->groupId);
        }
    }
}

void FriendWidget::setAsActiveChatroom()
{
    isActiveWidget = 1;

    QFont small;
    small.setPixelSize(10);
    statusMessage.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::darkGray);
    statusMessage.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::black);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, Qt::white);
    this->setPalette(pal3);
}

void FriendWidget::setAsInactiveChatroom()
{
    isActiveWidget = 0;

    QFont small;
    small.setPixelSize(10);
    statusMessage.setFont(small);
    QPalette pal;
    pal.setColor(QPalette::WindowText,Qt::gray);
    statusMessage.setPalette(pal);
    QPalette pal2;
    pal2.setColor(QPalette::WindowText,Qt::white);
    name.setPalette(pal2);
    QPalette pal3;
    pal3.setColor(QPalette::Background, QColor(65,65,65,255));
    this->setPalette(pal3);
}

void FriendWidget::updateStatusLight()
{
    Friend* f = FriendList::findFriend(friendId);
    Status status = f->friendStatus;

    if (status == Status::Online && f->hasNewEvents == 0)
        statusPic.setPixmap(QPixmap(":img/status/dot_online.png"));
    else if (status == Status::Online && f->hasNewEvents == 1)
        statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.png"));
    else if (status == Status::Away && f->hasNewEvents == 0)
        statusPic.setPixmap(QPixmap(":img/status/dot_idle.png"));
    else if (status == Status::Away && f->hasNewEvents == 1)
        statusPic.setPixmap(QPixmap(":img/status/dot_idle_notification.png"));
    else if (status == Status::Busy && f->hasNewEvents == 0)
        statusPic.setPixmap(QPixmap(":img/status/dot_busy.png"));
    else if (status == Status::Busy && f->hasNewEvents == 1)
        statusPic.setPixmap(QPixmap(":img/status/dot_busy_notification.png"));
    else if (status == Status::Offline && f->hasNewEvents == 0)
        statusPic.setPixmap(QPixmap(":img/status/dot_away.png"));
    else if (status == Status::Offline && f->hasNewEvents == 1)
        statusPic.setPixmap(QPixmap(":img/status/dot_away_notification.png"));
}

void FriendWidget::setChatForm(Ui::MainWindow &ui)
{
    Friend* f = FriendList::findFriend(friendId);
    f->chatForm->show(ui);
}

void FriendWidget::resetEventFlags()
{
    Friend* f = FriendList::findFriend(friendId);
    f->hasNewEvents = 0;
}

void FriendWidget::onAvatarChange(int FriendId, const QPixmap& pic)
{
    if (FriendId != friendId)
        return;

    isDefaultAvatar = false;
    avatar->setPixmap(pic);
}

void FriendWidget::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        dragStartPos = ev->pos();
}

void FriendWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if (!(ev->buttons() & Qt::LeftButton))
        return;

    if ((dragStartPos - ev->pos()).manhattanLength() > QApplication::startDragDistance())
    {
        QDrag* drag = new QDrag(this);
        QMimeData* mdata = new QMimeData;
        mdata->setData("friend", QString::number(friendId).toLatin1());

        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());

        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}
