/*
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

#include "friendwidget.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "groupwidget.h"
#include "circlewidget.h"
#include "friendlistwidget.h"
#include "src/friendlist.h"
#include "src/friend.h"
#include "src/core/core.h"
#include "form/chatform.h"
#include "maskablepixmapwidget.h"
#include "contentdialog.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/style.h"
#include "src/persistence/settings.h"
#include "src/widget/widget.h"
#include "src/widget/about/aboutuser.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QBitmap>
#include <QFileDialog>
#include <QDebug>
#include <QInputDialog>
#include <QCollator>
#include <cassert>

FriendWidget::FriendWidget(int FriendId, QString id)
    : friendId(FriendId)
    , isDefaultAvatar{true}
    , historyLoaded{false}
{
    avatar->setPixmap(QPixmap(":/img/contact.svg"), Qt::transparent);
    statusPic.setPixmap(QPixmap(":/img/status/dot_offline.svg"));
    statusPic.setMargin(3);
    nameLabel->setText(id);
    nameLabel->setTextFormat(Qt::PlainText);
    connect(nameLabel, &CroppingLabel::editFinished, this, &FriendWidget::setAlias);
    statusMessageLabel->setTextFormat(Qt::PlainText);
}

void FriendWidget::contextMenuEvent(QContextMenuEvent * event)
{
    if (!active)
        setBackgroundRole(QPalette::Highlight);

    installEventFilter(this); // Disable leave event.

    QPoint pos = event->globalPos();
    ToxId id = FriendList::findFriend(friendId)->getToxId();
    QString dir = Settings::getInstance().getAutoAcceptDir(id);
    QMenu menu;
    QAction* openChatWindow = nullptr;
    QAction* removeChatWindow = nullptr;

    ContentDialog* contentDialog = ContentDialog::getFriendDialog(friendId);
    bool notAlone = contentDialog != nullptr && contentDialog->chatroomWidgetCount() > 1;

    if (contentDialog == nullptr || notAlone)
        openChatWindow = menu.addAction(tr("Open chat in new window"));

    if (contentDialog->hasFriendWidget(friendId, this))
        removeChatWindow = menu.addAction(tr("Remove chat from this window"));

    menu.addSeparator();
    QMenu* inviteMenu = menu.addMenu(tr("Invite to group","Menu to invite a friend to a groupchat"));
    QMap<QAction*, Group*> groupActions;

    for (Group* group : GroupList::getAllGroups())
    {
        QAction* groupAction = inviteMenu->addAction(group->getGroupWidget()->getName());
        groupActions[groupAction] =  group;
    }

    if (groupActions.isEmpty())
        inviteMenu->setEnabled(false);

    int circleId = Settings::getInstance().getFriendCircleID(FriendList::findFriend(friendId)->getToxId());
    CircleWidget *circleWidget = CircleWidget::getFromID(circleId);

    QMenu* circleMenu = nullptr;
    QAction* newCircleAction = nullptr;
    QAction *removeCircleAction = nullptr;
    QMap<QAction*, int> circleActions;

    FriendListWidget *friendList;

    if (circleWidget == nullptr)
        friendList = dynamic_cast<FriendListWidget*>(FriendList::findFriend(friendId)->getFriendWidget()->parentWidget());
    else
        friendList = dynamic_cast<FriendListWidget*>(circleWidget->parentWidget());

    circleMenu = menu.addMenu(tr("Move to circle...", "Menu to move a friend into a different circle"));

    newCircleAction = circleMenu->addAction(tr("To new circle"));

    if (circleId != -1)
        removeCircleAction = circleMenu->addAction(tr("Remove from circle '%1'").arg(Settings::getInstance().getCircleName(circleId)));

    circleMenu->addSeparator();

    QList<QAction*> circleActionList;

    for (int i = 0; i < Settings::getInstance().getCircleCount(); ++i)
    {
        if (i != circleId)
        {
            circleActionList.push_back(new QAction(tr("Move  to circle \"%1\"").arg(Settings::getInstance().getCircleName(i)), circleMenu));
            circleActions[circleActionList.back()] = i;
        }
    }

    std::sort(circleActionList.begin(), circleActionList.end(), [](const QAction* lhs, const QAction* rhs) -> bool
    {
        QCollator collator;
        collator.setNumericMode(true);
        return collator.compare(lhs->text(), rhs->text()) < 0;
    });

    circleMenu->addActions(circleActionList);

    QAction* setAlias = menu.addAction(tr("Set alias..."));

    menu.addSeparator();
    QAction* autoAccept = menu.addAction(tr("Auto accept files from this friend", "context menu entry"));
    autoAccept->setCheckable(true);
    autoAccept->setChecked(!dir.isEmpty());
    menu.addSeparator();

    QAction* removeFriendAction = nullptr;

    if (contentDialog == nullptr || !contentDialog->hasFriendWidget(friendId, this))
        removeFriendAction = menu.addAction(tr("Remove friend", "Menu to remove the friend from our friendlist"));

    menu.addSeparator();
    QAction* aboutWindow = menu.addAction(tr("Show details"));

    QAction* selectedItem = menu.exec(pos);

    removeEventFilter(this);

    if (!active)
        setBackgroundRole(QPalette::Window);

    if (selectedItem)
    {
        if (selectedItem == setAlias)
        {
            nameLabel->editBegin();
        }
        else if (selectedItem == removeFriendAction)
        {
            emit removeFriend(friendId);
            return;
        }
        else if (selectedItem == openChatWindow)
        {
            emit chatroomWidgetClicked(this, true);
            return;
        }
        else if (selectedItem == removeChatWindow)
        {
            ContentDialog* contentDialog = ContentDialog::getFriendDialog(friendId);
            contentDialog->removeFriend(friendId);
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
            else if (autoAccept->isChecked())
            {
                dir = QFileDialog::getExistingDirectory(0, tr("Choose an auto accept directory","popup title"), dir);
                autoAccept->setChecked(true);
                qDebug() << "setting auto accept dir for" << friendId << "to" << dir;
                Settings::getInstance().setAutoAcceptDir(id, dir);
            }
        }
        else if (selectedItem == aboutWindow) {
            AboutUser *aboutUser = new AboutUser(id, this);
            aboutUser->setFriend(FriendList::findFriend(friendId));
            aboutUser->show();
        }
        else if (selectedItem == newCircleAction)
        {
            if (circleWidget != nullptr)
                circleWidget->updateStatus();

            if (friendList != nullptr)
                friendList->addCircleWidget(FriendList::findFriend(friendId)->getFriendWidget());
            else
                Settings::getInstance().setFriendCircleID(id, Settings::getInstance().addCircle());
        }
        else if (groupActions.contains(selectedItem))
        {
            Group* group = groupActions[selectedItem];
            Core::getInstance()->groupInviteFriend(friendId, group->getGroupId());
        }
        else if (removeCircleAction != nullptr && selectedItem == removeCircleAction)
        {
            if (friendList != nullptr)
                friendList->moveWidget(FriendList::findFriend(friendId)->getFriendWidget(), FriendList::findFriend(friendId)->getStatus(), true);
            else
                Settings::getInstance().setFriendCircleID(id, -1);

            if (circleWidget)
            {
                circleWidget->updateStatus();
                Widget::getInstance()->searchCircle(circleWidget);
            }
        }
        else if (circleActions.contains(selectedItem))
        {
            CircleWidget* circle = CircleWidget::getFromID(circleActions[selectedItem]);

            if (circle != nullptr)
            {
                circle->addFriendWidget(FriendList::findFriend(friendId)->getFriendWidget(), FriendList::findFriend(friendId)->getStatus());
                circle->setExpanded(true);
                Widget::getInstance()->searchCircle(circle);
                Settings::getInstance().savePersonal();
            }
            else
                Settings::getInstance().setFriendCircleID(id, circleActions[selectedItem]);

            if (circleWidget != nullptr)
            {
                circleWidget->updateStatus();
                Widget::getInstance()->searchCircle(circleWidget);
            }
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

    if (f->getEventFlag())
    {
        CircleWidget* circleWidget = CircleWidget::getFromID(Settings::getInstance().getFriendCircleID(FriendList::findFriend(friendId)->getToxId()));
        if (circleWidget != nullptr)
            circleWidget->setExpanded(true);

        Widget::getInstance()->updateFriendActivity(FriendList::findFriend(friendId));
    }

    if (!f->getEventFlag())
        statusPic.setMargin(3);
    else
        statusPic.setMargin(0);
}

QString FriendWidget::getStatusString() const
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

Friend* FriendWidget::getFriend() const
{
    return FriendList::findFriend(friendId);
}

void FriendWidget::search(const QString &searchString, bool hide)
{
    searchName(searchString, hide);
    CircleWidget* circleWidget = CircleWidget::getFromID(Settings::getInstance().getFriendCircleID(FriendList::findFriend(friendId)->getToxId()));
    if (circleWidget != nullptr)
        circleWidget->search(searchString);
}

bool FriendWidget::chatFormIsSet(bool focus) const
{
    Friend* f = FriendList::findFriend(friendId);
    return ContentDialog::existsFriendWidget(friendId, focus) || f->getChatForm()->isVisible();
}

void FriendWidget::setChatForm(ContentLayout* contentLayout)
{
    Friend* f = FriendList::findFriend(friendId);
    f->getChatForm()->show(contentLayout);
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
        avatar->setPixmap(QPixmap(":/img/contact_dark.svg"), Qt::transparent);
    else
        avatar->setPixmap(QPixmap(":/img/contact.svg"), Qt::transparent);
}

void FriendWidget::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        dragStartPos = ev->pos();

    GenericChatroomWidget::mousePressEvent(ev);
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
    QString alias = _alias.left(128); // same as TOX_MAX_NAME_LENGTH
    Friend* f = FriendList::findFriend(friendId);
    f->setAlias(alias);
    Settings::getInstance().setFriendAlias(f->getToxId(), alias);
    Settings::getInstance().savePersonal();
}
