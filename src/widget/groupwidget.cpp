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

#include <QPalette>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QApplication>
#include <QDrag>

#include "contentdialog.h"
#include "form/groupchatform.h"
#include "maskablepixmapwidget.h"
#include "src/core/core.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "src/persistence/settings.h"
#include "src/widget/friendwidget.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include "tool/croppinglabel.h"

GroupWidget::GroupWidget(int GroupId, QString Name)
    : groupId{GroupId}
{
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()));
    statusPic.setPixmap(QPixmap(":img/status/dot_online.svg"));
    statusPic.setMargin(3);
    nameLabel->setText(Name);

    const Settings& s = Settings::getInstance();
    connect(&s, &Settings::compactLayoutChanged,
            this, &GroupWidget::onCompactLayoutChanged);

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
    Translator::registerHandler(std::bind(&GroupWidget::retranslateUi, this), this);
}

GroupWidget::~GroupWidget()
{
    Translator::unregister(this);
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

    if (selectedItem)
    {
        if (selectedItem == quitGroup)
        {
            emit removeGroup(groupId);
        }
        else if (selectedItem == openChatWindow)
        {
            emit chatroomWidgetClicked(this, true);
            return;
        }
        else if (selectedItem == removeChatWindow)
        {
            ContentDialog* contentDialog = ContentDialog::getGroupDialog(groupId);
            contentDialog->removeGroup(groupId);
            return;
        }
        else if (selectedItem == setTitle)
        {
            editName();
        }
    }
}

void GroupWidget::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        dragStartPos = ev->pos();

    GenericChatroomWidget::mousePressEvent(ev);
}

void GroupWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if (!(ev->buttons() & Qt::LeftButton))
        return;

    if ((dragStartPos - ev->pos()).manhattanLength() > QApplication::startDragDistance())
    {
        QMimeData* mdata = new QMimeData;
        mdata->setText(getGroup()->getName());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void GroupWidget::onUserListChanged()
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
    {
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

QString GroupWidget::getStatusString() const
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

Group* GroupWidget::getGroup() const
{
    return GroupList::findGroup(groupId);
}

bool GroupWidget::chatFormIsSet(bool focus) const
{
    (void)focus;
    Group* g = GroupList::findGroup(groupId);
    return ContentDialog::existsGroupWidget(groupId, focus) || g->getChatForm()->isVisible();
}

void GroupWidget::setChatForm()
{
    Group* g = GroupList::findGroup(groupId);
    g->getChatForm()->show();
}

void GroupWidget::resetEventFlags()
{
    Group* g = GroupList::findGroup(groupId);
    g->setEventFlag(false);
    g->setMentionedFlag(false);
}

void GroupWidget::dragEnterEvent(QDragEnterEvent *ev)
{
    ToxId toxId = ToxId(ev->mimeData()->text());
    Friend *frnd = FriendList::findFriend(toxId);
    if (frnd)
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
    ToxId toxId = ToxId(ev->mimeData()->text());
    Friend *frnd = FriendList::findFriend(toxId);
    if (!frnd)
        return;

    int friendId = frnd->getFriendID();
    Core::getInstance()->groupInviteFriend(friendId, groupId);

    if (!active)
        setBackgroundRole(QPalette::Window);
}

void GroupWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}

void GroupWidget::retranslateUi()
{
    Group* g = GroupList::findGroup(groupId);
    if (g)
    {
        int peersCount = g->getPeersCount();
        if (peersCount == 1)
            statusMessageLabel->setText(tr("1 user in chat"));
        else
            statusMessageLabel->setText(tr("%1 users in chat").arg(peersCount));
    }
}
