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
#include "src/core/core.h"
#include "form/chatform.h"
#include "maskablepixmapwidget.h"
#include "croppinglabel.h"
#include "src/misc/style.h"
#include "src/misc/settings.h"
#include "src/widget/widget.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QBitmap>
#include <QFileDialog>
#include <QDebug>
#include <QInputDialog>

FriendWidget::FriendWidget(int FriendId, QString id)
    : friendId(FriendId)
    , isDefaultAvatar{true}
    , historyLoaded{false}
{
    avatar->setPixmap(QPixmap(":img/contact.svg"), Qt::transparent);
    statusPic.setPixmap(QPixmap(":img/status/dot_offline.svg"));
    statusPic.setMargin(3);
    nameLabel->setText(id);
    nameLabel->setTextFormat(Qt::PlainText);
    statusMessageLabel->setTextFormat(Qt::PlainText);
}

void FriendWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    ToxID id = FriendList::findFriend(friendId)->getToxID();
    QString dir = Settings::getInstance().getAutoAcceptDir(id);
    QMenu menu;
    QMenu* inviteMenu = menu.addMenu(tr("Invite to group","Menu to invite a friend to a groupchat"));
    QAction* copyId = menu.addAction(tr("Copy friend ID","Menu to copy the Tox ID of that friend"));
    QMap<QAction*, Group*> groupActions;
    
    for (Group* group : GroupList::getAllGroups())
    {
        QAction* groupAction = inviteMenu->addAction(group->getGroupWidget()->getName());
        groupActions[groupAction] =  group;
    }
    
    if (groupActions.isEmpty())
        inviteMenu->setEnabled(false);
    
    QAction* setAlias = menu.addAction(tr("Set alias..."));

    menu.addSeparator();
    QAction* autoAccept = menu.addAction(tr("Auto accept files from this friend", "context menu entry"));
    autoAccept->setCheckable(true);
    autoAccept->setChecked(!dir.isEmpty());
    menu.addSeparator();
    
    QAction* removeFriendAction = menu.addAction(tr("Remove friend", "Menu to remove the friend from our friendlist"));

    QAction* selectedItem = menu.exec(pos);
    if (selectedItem)
    {
        if (selectedItem == copyId)
        {
            emit copyFriendIdToClipboard(friendId);
            return;
        } else if (selectedItem == setAlias)
        {
            setFriendAlias();
        }
        else if (selectedItem == removeFriendAction)
        {
            emit removeFriend(friendId);
            return;
        }
        else if (selectedItem == autoAccept)
        {
            if (!autoAccept->isChecked())
            {
                qDebug() << "not checked";
                dir = QDir::homePath();
                autoAccept->setChecked(false);
                Settings::getInstance().setAutoAcceptDir(id, "");
            }
            
            if (autoAccept->isChecked())
            {
                dir = QFileDialog::getExistingDirectory(0, tr("Choose an auto accept directory","popup title"), dir);
                autoAccept->setChecked(true);
                qDebug() << "FriendWidget: setting auto accept dir for" << friendId << "to" << dir;
                Settings::getInstance().setAutoAcceptDir(id, dir);
            }
        }
        else if (groupActions.contains(selectedItem))
        {
            Group* group = groupActions[selectedItem];
            Core::getInstance()->groupInviteFriend(friendId, group->getGroupId());
        }
    }
}

void FriendWidget::setAsActiveChatroom()
{
    setActive(true);

    if (isDefaultAvatar)
        avatar->setPixmap(QPixmap(":img/contact_dark.svg"), Qt::transparent);
}

void FriendWidget::setAsInactiveChatroom()
{
    setActive(false);

    if (isDefaultAvatar)
        avatar->setPixmap(QPixmap(":img/contact.svg"), Qt::transparent);
}

void FriendWidget::updateStatusLight()
{
    Friend* f = FriendList::findFriend(friendId);
    Status status = f->getStatus();

    if (status == Status::Online && f->getEventFlag() == 0)
        statusPic.setPixmap(QPixmap(":img/status/dot_online.svg"));
    else if (status == Status::Online && f->getEventFlag() == 1)
        statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.svg"));
    else if (status == Status::Away && f->getEventFlag() == 0)
        statusPic.setPixmap(QPixmap(":img/status/dot_away.svg"));
    else if (status == Status::Away && f->getEventFlag() == 1)
        statusPic.setPixmap(QPixmap(":img/status/dot_away_notification.svg"));
    else if (status == Status::Busy && f->getEventFlag() == 0)
        statusPic.setPixmap(QPixmap(":img/status/dot_busy.svg"));
    else if (status == Status::Busy && f->getEventFlag() == 1)
        statusPic.setPixmap(QPixmap(":img/status/dot_busy_notification.svg"));
    else if (status == Status::Offline && f->getEventFlag() == 0)
        statusPic.setPixmap(QPixmap(":img/status/dot_offline.svg"));
    else if (status == Status::Offline && f->getEventFlag() == 1)
        statusPic.setPixmap(QPixmap(":img/status/dot_offline_notification.svg"));

    if (!f->getEventFlag())
        statusPic.setMargin(3);
    else
        statusPic.setMargin(0);
}

QString FriendWidget::getStatusString()
{
    Friend* f = FriendList::findFriend(friendId);
    Status status = f->getStatus();

    if (f->getEventFlag() == 1)
        return tr("New message");
    else if (status == Status::Online)
        return tr("Online");
    else if (status == Status::Away)
        return tr("Away");
    else if (status == Status::Busy)
        return tr("Busy");
    else if (status == Status::Offline)
        return tr("Offline");
    return QString::null;
}

void FriendWidget::setChatForm(Ui::MainWindow &ui)
{
    Friend* f = FriendList::findFriend(friendId);
    f->getChatForm()->show(ui);
}

void FriendWidget::resetEventFlags()
{
    Friend* f = FriendList::findFriend(friendId);
    f->setEventFlag(false);
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
        avatar->setPixmap(QPixmap(":img/contact_dark.svg"), Qt::transparent);
    else
        avatar->setPixmap(QPixmap(":img/contact.svg"), Qt::transparent);
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

void FriendWidget::setAlias(const QString& _alias)
{
    QString alias = _alias.trimmed();
    alias.remove(QRegExp("[\\t\\n\\v\\f\\r\\x0000]")); // we should really treat regular names this way as well (*ahem* zetok)
    alias = alias.left(128); // same as TOX_MAX_NAME_LENGTH
    Friend* f = FriendList::findFriend(friendId);
    f->setAlias(alias);
    Settings::getInstance().setFriendAlias(f->getToxID(), alias);
    hide();
    show();
}

void FriendWidget::setFriendAlias()
{
    bool ok;
    Friend* f = FriendList::findFriend(friendId);

    QString alias = QInputDialog::getText(nullptr, tr("User alias"), tr("You can also set this by clicking the chat form name.\nAlias:"), QLineEdit::Normal,
                                          f->getDisplayedName(), &ok);

    if (ok)
        setAlias(alias);
}
