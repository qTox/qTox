/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMenu>
#include <QMimeData>
#include <QPalette>

#include "contentdialog.h"
#include "maskablepixmapwidget.h"
#include "form/groupchatform.h"
#include "src/core/core.h"
#include "src/model/friend.h"
#include "src/friendlist.h"
#include "src/model/group.h"
#include "src/grouplist.h"
#include "src/widget/friendwidget.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include "tool/croppinglabel.h"

GroupWidget::GroupWidget(int groupId, const QString& name, bool compact)
    : GenericChatroomWidget(compact)
    , groupId{groupId}
{
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()));
    statusPic.setPixmap(QPixmap(":img/status/online.svg"));
    statusPic.setMargin(3);
    nameLabel->setText(name);

    updateUserCount();
    setAcceptDrops(true);

    Group* g = GroupList::findGroup(groupId);
    connect(g, &Group::titleChanged, this, &GroupWidget::updateTitle);
    connect(g, &Group::userListChanged, this, &GroupWidget::updateUserCount);
    connect(nameLabel, &CroppingLabel::editFinished, this, &GroupWidget::setTitle);
    Translator::registerHandler(std::bind(&GroupWidget::retranslateUi, this), this);
}

GroupWidget::~GroupWidget()
{
    Translator::unregister(this);
}

void GroupWidget::setTitle(const QString& newName)
{
    Group* g = GroupList::findGroup(groupId);
    g->setName(newName);
}

void GroupWidget::updateTitle(uint32_t groupId, const QString& author, const QString& newName)
{
    Q_UNUSED(groupId);
    Q_UNUSED(author);
    nameLabel->setText(newName);
}

void GroupWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!active)
        setBackgroundRole(QPalette::Highlight);

    installEventFilter(this); // Disable leave event.

    QMenu menu(this);

    QAction* openChatWindow = nullptr;
    QAction* removeChatWindow = nullptr;

    ContentDialog* contentDialog = ContentDialog::getGroupDialog(groupId);
    bool notAlone = contentDialog != nullptr && contentDialog->chatroomWidgetCount() > 1;

    if (contentDialog == nullptr || notAlone)
        openChatWindow = menu.addAction(tr("Open chat in new window"));

    if (contentDialog && contentDialog->hasGroupWidget(groupId, this))
        removeChatWindow = menu.addAction(tr("Remove chat from this window"));

    menu.addSeparator();

    QAction* setTitle = menu.addAction(tr("Set title..."));
    QAction* quitGroup = menu.addAction(tr("Quit group", "Menu to quit a groupchat"));

    QAction* selectedItem = menu.exec(event->globalPos());

    removeEventFilter(this);

    if (!active)
        setBackgroundRole(QPalette::Window);

    if (!selectedItem) {
        return;
    }

    if (selectedItem == quitGroup) {
        emit removeGroup(groupId);
    } else if (selectedItem == openChatWindow) {
        emit newWindowOpened(this);
    } else if (selectedItem == removeChatWindow) {
        ContentDialog* contentDialog = ContentDialog::getGroupDialog(groupId);
        contentDialog->removeGroup(groupId);
    } else if (selectedItem == setTitle) {
        editName();
    }
}

void GroupWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton)
        dragStartPos = ev->pos();

    GenericChatroomWidget::mousePressEvent(ev);
}

void GroupWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (!(ev->buttons() & Qt::LeftButton))
        return;

    if ((dragStartPos - ev->pos()).manhattanLength() > QApplication::startDragDistance()) {
        QMimeData* mdata = new QMimeData;
        mdata->setText(getGroup()->getName());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void GroupWidget::updateUserCount()
{
    Group* g = GroupList::findGroup(groupId);
    if (g) {
        int peersCount = g->getPeersCount();
        if (peersCount == 1)
            statusMessageLabel->setText(tr("1 user in chat"));
        else
            statusMessageLabel->setText(tr("%1 users in chat").arg(peersCount));
    }
}

void GroupWidget::setAsActiveChatroom()
{
    setActive(true);
    avatar->setPixmap(Style::scaleSvgImage(":img/group_dark.svg", avatar->width(), avatar->height()));
}

void GroupWidget::setAsInactiveChatroom()
{
    setActive(false);
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()));
}

void GroupWidget::updateStatusLight()
{
    Group* g = GroupList::findGroup(groupId);

    if (!g->getEventFlag()) {
        statusPic.setPixmap(QPixmap(":img/status/online.svg"));
        statusPic.setMargin(3);
    } else {
        statusPic.setPixmap(QPixmap(":img/status/online_notification.svg"));
        statusPic.setMargin(0);
    }
}

QString GroupWidget::getStatusString() const
{
    Group* g = GroupList::findGroup(groupId);

    if (!g->getEventFlag())
        return "Online";
    else
        return "New Message";
}

void GroupWidget::editName()
{
    nameLabel->editBegin();
}

Group* GroupWidget::getGroup() const
{
    return GroupList::findGroup(groupId);
}

void GroupWidget::resetEventFlags()
{
    Group* g = GroupList::findGroup(groupId);
    g->setEventFlag(false);
    g->setMentionedFlag(false);
}

void GroupWidget::dragEnterEvent(QDragEnterEvent* ev)
{
    ToxId toxId = ToxId(ev->mimeData()->text());
    Friend* frnd = FriendList::findFriend(toxId.getPublicKey());
    if (frnd)
        ev->acceptProposedAction();

    if (!active)
        setBackgroundRole(QPalette::Highlight);
}

void GroupWidget::dragLeaveEvent(QDragLeaveEvent*)
{
    if (!active)
        setBackgroundRole(QPalette::Window);
}

void GroupWidget::dropEvent(QDropEvent* ev)
{
    ToxId toxId = ToxId(ev->mimeData()->text());
    Friend* frnd = FriendList::findFriend(toxId.getPublicKey());
    if (!frnd)
        return;

    int friendId = frnd->getId();
    if (frnd->getStatus() != Status::Offline) {
        Core::getInstance()->groupInviteFriend(friendId, groupId);
    }

    if (!active)
        setBackgroundRole(QPalette::Window);
}

void GroupWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}

void GroupWidget::retranslateUi()
{
    updateUserCount();
}
