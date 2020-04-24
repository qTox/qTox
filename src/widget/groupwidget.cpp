/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "maskablepixmapwidget.h"
#include "form/groupchatform.h"
#include "src/core/core.h"
#include "src/model/friend.h"
#include "src/friendlist.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/grouplist.h"
#include "src/widget/friendwidget.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"
#include "tool/croppinglabel.h"

GroupWidget::GroupWidget(std::shared_ptr<GroupChatroom> chatroom, bool compact)
    : GenericChatroomWidget(compact)
    , groupId{chatroom->getGroup()->getPersistentId()}
    , chatroom{chatroom}
{
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()));
    statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Online)));
    statusPic.setMargin(3);

    Group* g = chatroom->getGroup();
    nameLabel->setText(g->getName());

    updateUserCount(g->getPeersCount());
    setAcceptDrops(true);

    connect(g, &Group::titleChanged, this, &GroupWidget::updateTitle);
    connect(g, &Group::numPeersChanged, this, &GroupWidget::updateUserCount);
    connect(nameLabel, &CroppingLabel::editFinished, g, &Group::setName);
    Translator::registerHandler(std::bind(&GroupWidget::retranslateUi, this), this);
}

GroupWidget::~GroupWidget()
{
    Translator::unregister(this);
}

void GroupWidget::updateTitle(const QString& author, const QString& newName)
{
    Q_UNUSED(author)
    nameLabel->setText(newName);
}

void GroupWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!active) {
        setBackgroundRole(QPalette::Highlight);
    }

    installEventFilter(this); // Disable leave event.

    QMenu menu(this);

    QAction* openChatWindow = nullptr;
    if (chatroom->possibleToOpenInNewWindow() ) {
        openChatWindow = menu.addAction(tr("Open chat in new window"));
    }

    QAction* removeChatWindow = nullptr;
    if (chatroom->canBeRemovedFromWindow()) {
        removeChatWindow = menu.addAction(tr("Remove chat from this window"));
    }

    menu.addSeparator();

    QAction* setTitle = menu.addAction(tr("Set title..."));
    QAction* quitGroup = menu.addAction(tr("Quit group", "Menu to quit a groupchat"));

    QAction* selectedItem = menu.exec(event->globalPos());

    removeEventFilter(this);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }

    if (!selectedItem) {
        return;
    }

    if (selectedItem == quitGroup) {
        emit removeGroup(groupId);
    } else if (selectedItem == openChatWindow) {
        emit newWindowOpened(this);
    } else if (selectedItem == removeChatWindow) {
        chatroom->removeGroupFromDialogs();
    } else if (selectedItem == setTitle) {
        editName();
    }
}

void GroupWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        dragStartPos = ev->pos();
    }

    GenericChatroomWidget::mousePressEvent(ev);
}

void GroupWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (!(ev->buttons() & Qt::LeftButton)) {
        return;
    }

    if ((dragStartPos - ev->pos()).manhattanLength() > QApplication::startDragDistance()) {
        QMimeData* mdata = new QMimeData;
        const Group* group = getGroup();
        mdata->setText(group->getName());
        mdata->setData("groupId", group->getPersistentId().getByteArray());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void GroupWidget::updateUserCount(int numPeers)
{
    statusMessageLabel->setText(tr("%n user(s) in chat", "Number of users in chat", numPeers));
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
    Group* g = chatroom->getGroup();

    const bool event = g->getEventFlag();
    statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Online, event)));
    statusPic.setMargin(event ? 1 : 3);
}

QString GroupWidget::getStatusString() const
{
    if (chatroom->hasNewMessage()) {
        return tr("New Message");
    } else {
        return tr("Online");
    }
}

void GroupWidget::editName()
{
    nameLabel->editBegin();
}

// TODO: Remove
Group* GroupWidget::getGroup() const
{
    return chatroom->getGroup();
}

const Contact* GroupWidget::getContact() const
{
    return getGroup();
}

void GroupWidget::resetEventFlags()
{
    chatroom->resetEventFlags();
}

void GroupWidget::dragEnterEvent(QDragEnterEvent* ev)
{
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk pk{ev->mimeData()->data("toxPk")};
    if (chatroom->friendExists(pk)) {
        ev->acceptProposedAction();
    }

    if (!active) {
        setBackgroundRole(QPalette::Highlight);
    }
}

void GroupWidget::dragLeaveEvent(QDragLeaveEvent*)
{
    if (!active) {
        setBackgroundRole(QPalette::Window);
    }
}

void GroupWidget::dropEvent(QDropEvent* ev)
{
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk pk{ev->mimeData()->data("toxPk")};
    if (!chatroom->friendExists(pk)) {
        return;
    }

    chatroom->inviteFriend(pk);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }
}

void GroupWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}

void GroupWidget::retranslateUi()
{
    const Group* group = chatroom->getGroup();
    updateUserCount(group->getPeersCount());
}
