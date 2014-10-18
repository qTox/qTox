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
#include "src/group.h"
#include "src/grouplist.h"
#include "groupwidget.h"
#include "src/friendlist.h"
#include "src/friend.h"
#include "src/core.h"
#include "form/chatform.h"
#include "maskablepixmapwidget.h"
#include "croppinglabel.h"
#include "src/misc/style.h"
#include "src/misc/settings.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QBitmap>
#include <QFileDialog>
#include <QDebug>

FriendWidget::FriendWidget(int FriendId, QString id)
    : friendId(FriendId)
    , isDefaultAvatar{true}
{
    avatar->setPixmap(QPixmap(":img/contact.png"), Qt::transparent);
    statusPic.setPixmap(QPixmap(":img/status/dot_away.png"));
    nameLabel->setText(id);
}

void FriendWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    QString id = Core::getInstance()->getFriendAddress(friendId);
    QString dir = Settings::getInstance().getAutoAcceptDir(id);
    QMenu menu;
    QAction* copyId = menu.addAction(tr("Copy friend ID","Menu to copy the Tox ID of that friend"));
    QMenu* inviteMenu = menu.addMenu(tr("Invite in group","Menu to invite a friend in a groupchat"));
    QMap<QAction*, Group*> groupActions;
    for (Group* group : GroupList::groupList)
    {
        QAction* groupAction = inviteMenu->addAction(group->widget->getName());
        groupActions[groupAction] =  group;
    }
    if (groupActions.isEmpty())
        inviteMenu->setEnabled(false);
    QAction* autoAccept = menu.addAction(tr("Auto accept files from this friend", "context menu entry"));
    QAction* disableAutoAccept = menu.addAction(tr("Diasble auto accepting files", "context menu entry"));
    if (dir.isEmpty())
        disableAutoAccept->setEnabled(false);
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
        else if (selectedItem == autoAccept)
        {
            if (dir.isEmpty())
                dir = QDir::homePath();
            dir = QFileDialog::getExistingDirectory(0, tr("Choose an auto accept directory","popup title"), dir);
            if (!dir.isEmpty())
            {
                qDebug() << "FriendWidget: setting auto accept dir for" << friendId << "to" << dir;
                Settings::getInstance().setAutoAcceptDir(id, dir);
            }
        }
        else if (selectedItem == disableAutoAccept)
        {
            Settings::getInstance().setAutoAcceptDir(id, "");
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
    setActive(true);

    if (isDefaultAvatar)
        avatar->setPixmap(QPixmap(":img/contact_dark.png"), Qt::transparent);
}

void FriendWidget::setAsInactiveChatroom()
{
    setActive(false);

    if (isDefaultAvatar)
        avatar->setPixmap(QPixmap(":img/contact.png"), Qt::transparent);
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
    avatar->autopickBackground();
}

void FriendWidget::onAvatarRemoved(int FriendId)
{
    if (FriendId != friendId)
        return;

    isDefaultAvatar = true;

    if (isActive())
        avatar->setPixmap(QPixmap(":img/contact_dark.png"), Qt::transparent);
    else
        avatar->setPixmap(QPixmap(":img/contact.png"), Qt::transparent);
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
