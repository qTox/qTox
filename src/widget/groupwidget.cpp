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
#include "src/grouplist.h"
#include "src/group.h"
#include "src/persistence/settings.h"
#include "form/groupchatform.h"
#include "maskablepixmapwidget.h"
#include "src/widget/style.h"
#include "src/core/core.h"
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
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()), Qt::transparent);
    statusPic.setPixmap(QPixmap(":img/status/dot_online.svg"));
    statusPic.setMargin(3);
    nameLabel->setText(Name);

    Group* g = GroupList::findGroup(groupId);
    if (g)
        statusMessageLabel->setText(GroupWidget::tr("%1 users in chat").arg(g->getPeersCount()));
    else
        statusMessageLabel->setText(GroupWidget::tr("0 users in chat"));

    setAcceptDrops(true);

    connect(nameLabel, &CroppingLabel::textChanged, [this](const QString &newName, const QString &oldName)
    {
        Group* g = GroupList::findGroup(groupId);
        if (newName != oldName)
        {
            nameLabel->setText(oldName);
            emit renameRequested(newName);
            emit g->getChatForm()->groupTitleChanged(groupId, newName.left(128));
        }
        /* according to agilob:
         * “Moving mouse pointer over groupwidget results in CSS effect
         * mouse-over(?). Changing group title repaints only changed
         * element - title, the rest of the widget stays in the same CSS as it
         * was on mouse over. Repainting whole widget fixes style problem.”
         */
        this->repaint();
    });
}

void GroupWidget::contextMenuEvent(QContextMenuEvent * event)
{
    QPoint pos = event->globalPos();
    QMenu menu;
    QAction* setTitle = menu.addAction(tr("Set title..."));
    QAction* quitGroup = menu.addAction(tr("Quit group","Menu to quit a groupchat"));

    QAction* selectedItem = menu.exec(pos);
    if (selectedItem)
    {
        if (selectedItem == quitGroup)
        {
            emit removeGroup(groupId);
        }
        else if (selectedItem == setTitle)
        {
            nameLabel->editStart();
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

void GroupWidget::rename()
{
    nameLabel->editStart();
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
    setAttribute(Qt::WA_UnderMouse, true); // Simulate hover.
    Style::repolish(this);
}

void GroupWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    setAttribute(Qt::WA_UnderMouse, false);
    Style::repolish(this);
}

void GroupWidget::dropEvent(QDropEvent *ev)
{
    if (ev->mimeData()->hasFormat("friend"))
    {
        int friendId = ev->mimeData()->data("friend").toInt();
        Core::getInstance()->groupInviteFriend(friendId, groupId);

        setAttribute(Qt::WA_UnderMouse, false);
        Style::repolish(this);
    }
}

void GroupWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}
