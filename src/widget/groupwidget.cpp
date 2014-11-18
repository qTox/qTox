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
#include "src/grouplist.h"
#include "src/group.h"
#include "src/misc/settings.h"
#include "form/groupchatform.h"
#include "maskablepixmapwidget.h"
#include "src/misc/style.h"
#include "src/core.h"
#include <QPalette>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QInputDialog>

#include "ui_mainwindow.h"

GroupWidget::GroupWidget(int GroupId, QString Name)
    : groupId{GroupId}
{
    avatar->setPixmap(QPixmap(":img/group.png"), Qt::transparent);
    statusPic.setPixmap(QPixmap(":img/status/dot_online.png"));
    nameLabel->setText(Name);

    Group* g = GroupList::findGroup(groupId);
    if (g)
        statusMessageLabel->setText(GroupWidget::tr("%1 users in chat").arg(g->peers.size()));
    else
        statusMessageLabel->setText(GroupWidget::tr("0 users in chat"));

    setAcceptDrops(true);
}

void GroupWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    QMenu menu;
    QAction* quitGroup = menu.addAction(tr("Quit group","Menu to quit a groupchat"));
    QAction* setAlias = menu.addAction(tr("Set title..."));

    QAction* selectedItem = menu.exec(pos);
    if (selectedItem)
    {
        if (selectedItem == quitGroup)
            emit removeGroup(groupId);
        else if (selectedItem == setAlias)
        {
            bool ok;
            Group* g = GroupList::findGroup(groupId);

            QString alias = QInputDialog::getText(nullptr, tr("Group title"), tr("You can also set this by clicking the chat form name.\nTitle:"), QLineEdit::Normal,
                                          nameLabel->fullText(), &ok);

            if (ok && alias != nameLabel->fullText())
                emit g->chatForm->groupTitleChanged(groupId, alias.left(128));
        }
    }
}

void GroupWidget::onUserListChanged()
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
        statusMessageLabel->setText(tr("%1 users in chat").arg(g->nPeers));
    else
        statusMessageLabel->setText(tr("0 users in chat"));
}

void GroupWidget::setAsActiveChatroom()
{
    setActive(true);
    avatar->setPixmap(QPixmap(":img/group_dark.png"), Qt::transparent);
}

void GroupWidget::setAsInactiveChatroom()
{
    setActive(false);
    avatar->setPixmap(QPixmap(":img/group.png"), Qt::transparent);
}

void GroupWidget::updateStatusLight()
{
    Group *g = GroupList::findGroup(groupId);

    if (g->hasNewMessages == 0)
    {
        statusPic.setPixmap(QPixmap(":img/status/dot_online.png"));
    }
    else
    {
        if (g->userWasMentioned == 0)
            statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.png"));
        else
            statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.png"));
    }
}

void GroupWidget::setChatForm(Ui::MainWindow &ui)
{
    Group* g = GroupList::findGroup(groupId);
    g->chatForm->show(ui);
}

void GroupWidget::resetEventFlags()
{
    Group* g = GroupList::findGroup(groupId);
    g->hasNewMessages = 0;
    g->userWasMentioned = 0;
}

void GroupWidget::dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasFormat("friend"))
        ev->acceptProposedAction();
}

void GroupWidget::dropEvent(QDropEvent *ev)
{
    if (ev->mimeData()->hasFormat("friend"))
    {
        int friendId = ev->mimeData()->data("friend").toInt();
        Core::getInstance()->groupInviteFriend(friendId, groupId);
    }
}

void GroupWidget::keyPressEvent(QKeyEvent* ev)
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
        g->chatForm->keyPressEvent(ev);

}

void GroupWidget::keyReleaseEvent(QKeyEvent* ev)
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
        g->chatForm->keyReleaseEvent(ev);
}

void GroupWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}
