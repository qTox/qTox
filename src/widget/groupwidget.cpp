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

#include "groupwidget.h"
#include "maskablepixmapwidget.h"
#include "src/grouplist.h"
#include "src/group.h"
#include "form/groupchatform.h"
#include "src/widget/style.h"
#include "src/core/core.h"
#include "tool/croppinglabel.h"
#include <QPalette>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QMimeData>

GroupWidget::GroupWidget(int GroupId, QString Name)
    : groupId{GroupId}
{
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()), Qt::transparent);
    statusPic.setPixmap(QPixmap(":img/status/dot_online.svg"));
    statusPic.setMargin(3);
    nameLabel->setText(Name);

    onUserListChanged();

    setAcceptDrops(true);

    connect(nameLabel, &CroppingLabel::editFinished, [this](const QString &newName)
    {
        if (!newName.isEmpty())
        {
            Group* g = GroupList::findGroup(groupId);
            emit renameRequested(this, newName);
            emit g->getChatForm()->groupTitleChanged(groupId, newName.left(128));
        }
    });
}

void GroupWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!active)
        setBackgroundRole(QPalette::Highlight);

    installEventFilter(this); // Disable leave event.

    QMenu menu(this);
    QAction* setTitle = menu.addAction(tr("Set title..."));
    QAction* quitGroup = menu.addAction(tr("Quit group","Menu to quit a groupchat"));

    QAction* selectedItem = menu.exec(event->globalPos());

    removeEventFilter(this);

    if (!active)
        setBackgroundRole(QPalette::Window);

    if (selectedItem)
    {
        if (selectedItem == quitGroup)
        {
            emit removeGroup(groupId);
        }
        else if (selectedItem == setTitle)
        {
            editName();
        }
    }
}

void GroupWidget::onUserListChanged()
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
        statusMessageLabel->setText(tr("%1 users in chat").arg(g->getPeersCount()));
    else
        statusMessageLabel->setText(tr("0 users in chat"));
}

void GroupWidget::setAsActiveChatroom()
{
    setActive(true);
    avatar->setPixmap(Style::scaleSvgImage(":img/group_dark.svg", avatar->width(), avatar->height()), Qt::transparent);
}

void GroupWidget::setAsInactiveChatroom()
{
    setActive(false);
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()), Qt::transparent);
}

void GroupWidget::updateStatusLight()
{
    Group *g = GroupList::findGroup(groupId);

    if (!g->getEventFlag())
    {
        statusPic.setPixmap(QPixmap(":img/status/dot_online.svg"));
        statusPic.setMargin(3);
    }
    else
    {
        statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.svg"));
        statusPic.setMargin(0);
    }
}

QString GroupWidget::getStatusString()
{
    Group *g = GroupList::findGroup(groupId);

    if (!g->getEventFlag())
        return "Online";
    else
        return "New Message";
}

void GroupWidget::editName()
{
    nameLabel->editBegin();
}

void GroupWidget::setChatForm(Ui::MainWindow &ui)
{
    Group* g = GroupList::findGroup(groupId);
    g->getChatForm()->show(ui);
}

void GroupWidget::resetEventFlags()
{
    Group* g = GroupList::findGroup(groupId);
    g->setEventFlag(false);
    g->setMentionedFlag(false);
}

void GroupWidget::dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasFormat("friend"))
        ev->acceptProposedAction();

    if (!active)
        setBackgroundRole(QPalette::Highlight);
}

void GroupWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    if (!active)
        setBackgroundRole(QPalette::Window);
}

void GroupWidget::dropEvent(QDropEvent *ev)
{
    if (ev->mimeData()->hasFormat("friend"))
    {
        int friendId = ev->mimeData()->data("friend").toInt();
        Core::getInstance()->groupInviteFriend(friendId, groupId);

        if (!active)
            setBackgroundRole(QPalette::Window);
    }
}

void GroupWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}
