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

#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMenu>
#include <QMimeData>
#include <QPalette>

#include "src/core/core.h"
#include "src/friend.h"
#include "src/persistence/settings.h"
#include "src/widget/contentdialog.h"
#include "src/widget/form/groupchatform.h"
#include "src/widget/friendlistwidget.h"
#include "src/widget/friendwidget.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/translator.h"

GroupWidget::GroupWidget(Group* group, FriendListWidget* parent)
    : GenericChatroomWidget(parent)
    , g(group)
{
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()));
    statusPic.setPixmap(QPixmap(":img/status/dot_online.svg"));
    statusPic.setMargin(3);
    nameLabel->setText(g->getName());

    setAcceptDrops(true);

    const Settings& s = Settings::getInstance();
    connect(&s, &Settings::compactLayoutChanged,
            this, &GroupWidget::onCompactLayoutChanged);

    connect(nameLabel, &CroppingLabel::editFinished, [this](const QString &newName)
    {
        g->setName(newName);
    });

    const Core* core = Core::getInstance();
    connect(core, &Core::groupTitleChanged,
            this, [&](Group::ID groupId, const QString& author, const QString& title)
    {
        Q_UNUSED(author);

        if (groupId == g->getGroupId())
            setName(title);
    });
    connect(Group::notify(), &GroupNotify::userListChanged,
            this, &GroupWidget::onUserListChanged);

    Translator::registerHandler(std::bind(&GroupWidget::retranslateUi, this), this);

    onUserListChanged(*g, g->getPeersCount(), 0);
}

GroupWidget::~GroupWidget()
{
    Translator::unregister(this);
}

GenericChatroomWidget::Type GroupWidget::type() const
{
    return Type::Group;
}

void GroupWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!active)
        setBackgroundRole(QPalette::Highlight);

    installEventFilter(this); // Disable leave event.

    QMenu menu(this);

    QAction* openChatWindow = nullptr;
    QAction* removeChatWindow = nullptr;

    ContentDialog* contentDialog = ContentDialog::getGroupDialog(g->getGroupId());
    bool notAlone = contentDialog != nullptr && contentDialog->chatroomWidgetCount() > 1;

    if (contentDialog == nullptr || notAlone)
        openChatWindow = menu.addAction(tr("Open chat in new window"));

    if (contentDialog && contentDialog->hasGroupWidget(g->getGroupId(), this))
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
            emit removeGroup(g->getGroupId());
        }
        else if (selectedItem == openChatWindow)
        {
            emit chatroomWidgetClicked(this, true);
            return;
        }
        else if (selectedItem == removeChatWindow)
        {
            ContentDialog* contentDialog = ContentDialog::getGroupDialog(g->getGroupId());
            contentDialog->removeGroup(g->getGroupId());
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
        mdata->setText(g->getName());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void GroupWidget::onUserListChanged(const Group& group, int numPeers,
                                    quint8 change)
{
    Q_UNUSED(change);

    if (group.getGroupId() == g->getGroupId())
    {
        int n = numPeers;
        statusMessageLabel->setText(tr("%n user(s) in chat", "", n));
    }
}

void GroupWidget::setAsActiveChatroom(bool activate)
{
    setActive(activate);
    avatar->setPixmap(Style::scaleSvgImage(activate ? ":img/group_dark.svg"
                                                    : ":img/group.svg",
                                           avatar->width(), avatar->height()));
}

void GroupWidget::updateStatusLight()
{
    if (!hasNewMessages)
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

QString GroupWidget::getTitle() const
{
    QString statusStr = hasNewMessages ? tr("New Message")
                                       : Group::statusToString(g);
    return statusStr.isEmpty()
            ? GenericChatroomWidget::getTitle()
            : QStringLiteral(" (") + statusStr + QStringLiteral(")");
}

void GroupWidget::editName()
{
    nameLabel->editBegin();
}

bool GroupWidget::chatFormIsSet(bool focus) const
{
    return ContentDialog::groupWidgetExists(g->getGroupId(), focus);
}

void GroupWidget::resetEventFlags()
{
    hasNewMessages = false;
    userWasMentioned = false;
}

void GroupWidget::dragEnterEvent(QDragEnterEvent *ev)
{
    ToxId toxId = ToxId(ev->mimeData()->text());
    Friend frnd = Friend::get(toxId);
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
    Friend frnd = Friend::get(toxId);
    if (!frnd)
        return;

    Friend::ID friendId = frnd.getFriendId();
    Core::getInstance()->groupInviteFriend(friendId, g->getGroupId());

    if (!active)
        setBackgroundRole(QPalette::Window);
}

void GroupWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}

void GroupWidget::retranslateUi()
{
    if (g)
    {
        int peersCount = g->getPeersCount();
        if (peersCount == 1)
            statusMessageLabel->setText(tr("1 user in chat"));
        else
            statusMessageLabel->setText(tr("%1 users in chat").arg(peersCount));
    }
}
