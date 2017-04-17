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

#include "circlewidget.h"
#include "contentdialog.h"
#include "friendlistwidget.h"
#include "groupwidget.h"
#include "maskablepixmapwidget.h"

#include "src/core/core.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "src/persistence/settings.h"
#include "src/widget/about/aboutuser.h"
#include "src/widget/form/chatform.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/widget.h"

#include <QApplication>
#include <QBitmap>
#include <QCollator>
#include <QContextMenuEvent>
#include <QDebug>
#include <QDrag>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMimeData>

#include <cassert>

/**
 * @class FriendWidget
 *
 * Widget, which displays brief information about friend.
 * For example, used on friend list.
 * When you click should open the chat with friend. Widget has a context menu.
 */

FriendWidget::FriendWidget(int friendId, const QString& id, bool compact)
    : GenericChatroomWidget(compact)
    , friendId(friendId)
    , isDefaultAvatar{true}
    , historyLoaded{false}
{
    avatar->setPixmap(QPixmap(":/img/contact.svg"));
    statusPic.setPixmap(QPixmap(":/img/status/dot_offline.svg"));
    statusPic.setMargin(3);
    nameLabel->setText(id);
    nameLabel->setTextFormat(Qt::PlainText);
    connect(nameLabel, &CroppingLabel::editFinished, this, &FriendWidget::setAlias);
    statusMessageLabel->setTextFormat(Qt::PlainText);
}

/**
 * @brief FriendWidget::contextMenuEvent
 * @param event Describe a context menu event
 *
 * Default context menu event handler.
 * Redirect all event information to the signal.
 */
void FriendWidget::contextMenuEvent(QContextMenuEvent* event)
{
    emit contextMenuCalled(event);
}

/**
 * @brief FriendWidget::onContextMenuCalled
 * @param event Redirected from native contextMenuEvent
 *
 * Context menu handler. Always should be called to FriendWidget from FriendList
 */
void FriendWidget::onContextMenuCalled(QContextMenuEvent* event)
{
    if (!active)
        setBackgroundRole(QPalette::Highlight);

    installEventFilter(this); // Disable leave event.

    QPoint pos = event->globalPos();
    ToxPk id = FriendList::findFriend(friendId)->getPublicKey();
    QString dir = Settings::getInstance().getAutoAcceptDir(id);
    QMenu menu;
    QAction* openChatWindow = nullptr;
    QAction* removeChatWindow = nullptr;

    ContentDialog* contentDialog = ContentDialog::getFriendDialog(friendId);
    bool notAlone = contentDialog != nullptr && contentDialog->chatroomWidgetCount() > 1;

    if (contentDialog == nullptr || notAlone)
        openChatWindow = menu.addAction(tr("Open chat in new window"));

    if (contentDialog != nullptr && contentDialog->hasFriendWidget(friendId, this))
        removeChatWindow = menu.addAction(tr("Remove chat from this window"));

    menu.addSeparator();
    QMenu* inviteMenu =
        menu.addMenu(tr("Invite to group", "Menu to invite a friend to a groupchat"));
    inviteMenu->setEnabled(getFriend()->getStatus() != Status::Offline);
    QAction* newGroupAction = inviteMenu->addAction(tr("To new group"));
    inviteMenu->addSeparator();
    QMap<QAction*, Group*> groupActions;

    for (Group* group : GroupList::getAllGroups()) {
        int maxNameLen = 30;
        QString name = group->getGroupWidget()->getName();
        if (name.length() > maxNameLen) {
            name = name.left(maxNameLen).trimmed() + "..";
        }
        QAction* groupAction = inviteMenu->addAction(tr("Invite to group '%1'").arg(name));
        groupActions[groupAction] = group;
    }

    int circleId =
        Settings::getInstance().getFriendCircleID(FriendList::findFriend(friendId)->getPublicKey());
    CircleWidget* circleWidget = CircleWidget::getFromID(circleId);

    QMenu* circleMenu = nullptr;
    QAction* newCircleAction = nullptr;
    QAction* removeCircleAction = nullptr;
    QMap<QAction*, int> circleActions;

    FriendListWidget* friendList;

    if (circleWidget == nullptr)
        friendList = qobject_cast<FriendListWidget*>(this->parentWidget());
    else
        friendList = qobject_cast<FriendListWidget*>(circleWidget->parentWidget());

    circleMenu =
        menu.addMenu(tr("Move to circle...", "Menu to move a friend into a different circle"));

    newCircleAction = circleMenu->addAction(tr("To new circle"));

    if (circleId != -1)
        removeCircleAction = circleMenu->addAction(
            tr("Remove from circle '%1'").arg(Settings::getInstance().getCircleName(circleId)));

    circleMenu->addSeparator();

    QList<QAction*> circleActionList;

    for (int i = 0; i < Settings::getInstance().getCircleCount(); ++i) {
        if (i != circleId) {
            circleActionList.push_back(
                new QAction(tr("Move  to circle \"%1\"").arg(Settings::getInstance().getCircleName(i)),
                            circleMenu));
            circleActions[circleActionList.back()] = i;
        }
    }

    std::sort(circleActionList.begin(), circleActionList.end(),
              [](const QAction* lhs, const QAction* rhs) -> bool {
                  QCollator collator;
                  collator.setNumericMode(true);
                  return collator.compare(lhs->text(), rhs->text()) < 0;
              });

    circleMenu->addActions(circleActionList);

    QAction* setAlias = menu.addAction(tr("Set alias..."));

    menu.addSeparator();
    QAction* autoAccept =
        menu.addAction(tr("Auto accept files from this friend", "context menu entry"));
    autoAccept->setCheckable(true);
    autoAccept->setChecked(!dir.isEmpty());
    menu.addSeparator();

    QAction* removeFriendAction = nullptr;

    if (contentDialog == nullptr || !contentDialog->hasFriendWidget(friendId, this))
        removeFriendAction =
            menu.addAction(tr("Remove friend", "Menu to remove the friend from our friendlist"));

    menu.addSeparator();
    QAction* aboutWindow = menu.addAction(tr("Show details"));

    QAction* selectedItem = menu.exec(pos);

    removeEventFilter(this);

    if (!active)
        setBackgroundRole(QPalette::Window);

    if (!selectedItem)
        return;

    if (selectedItem == setAlias) {
        nameLabel->editBegin();
    } else if (selectedItem == removeFriendAction) {
        emit removeFriend(friendId);
    } else if (selectedItem == openChatWindow) {
        emit chatroomWidgetClicked(this, true);
    } else if (selectedItem == removeChatWindow) {
        ContentDialog* contentDialog = ContentDialog::getFriendDialog(friendId);
        contentDialog->removeFriend(friendId);
    } else if (selectedItem == autoAccept) {
        if (!autoAccept->isChecked()) {
            qDebug() << "not checked";
            dir = QDir::homePath();
            autoAccept->setChecked(false);
            Settings::getInstance().setAutoAcceptDir(id, "");
        } else if (autoAccept->isChecked()) {
            dir = QFileDialog::getExistingDirectory(0, tr("Choose an auto accept directory",
                                                          "popup title"),
                                                    dir, QFileDialog::DontUseNativeDialog);

            autoAccept->setChecked(true);
            qDebug() << "Setting auto accept dir for" << friendId << "to" << dir;
            Settings::getInstance().setAutoAcceptDir(id, dir);
        }
    } else if (selectedItem == aboutWindow) {
        AboutUser* aboutUser = new AboutUser(id, Widget::getInstance());
        aboutUser->setFriend(FriendList::findFriend(friendId));
        aboutUser->show();
    } else if (selectedItem == newGroupAction) {
        int groupId = Core::getInstance()->createGroup();
        Core::getInstance()->groupInviteFriend(friendId, groupId);
    } else if (selectedItem == newCircleAction) {
        if (circleWidget != nullptr)
            circleWidget->updateStatus();

        if (friendList != nullptr)
            friendList->addCircleWidget(this);
        else
            Settings::getInstance().setFriendCircleID(id, Settings::getInstance().addCircle());
    } else if (groupActions.contains(selectedItem)) {
        Group* group = groupActions[selectedItem];
        Core::getInstance()->groupInviteFriend(friendId, group->getGroupId());
    } else if (removeCircleAction != nullptr && selectedItem == removeCircleAction) {
        if (friendList)
            friendList->moveWidget(this, FriendList::findFriend(friendId)->getStatus(), true);
        else
            Settings::getInstance().setFriendCircleID(id, -1);

        if (circleWidget) {
            circleWidget->updateStatus();
            Widget::getInstance()->searchCircle(circleWidget);
        }
    } else if (circleActions.contains(selectedItem)) {
        CircleWidget* circle = CircleWidget::getFromID(circleActions[selectedItem]);

        if (circle) {
            circle->addFriendWidget(this, FriendList::findFriend(friendId)->getStatus());
            circle->setExpanded(true);
            Widget::getInstance()->searchCircle(circle);
            Settings::getInstance().savePersonal();
        } else {
            Settings::getInstance().setFriendCircleID(id, circleActions[selectedItem]);
        }

        if (circleWidget) {
            circleWidget->updateStatus();
            Widget::getInstance()->searchCircle(circleWidget);
        }
    }
}

void FriendWidget::setAsActiveChatroom()
{
    setActive(true);

    if (isDefaultAvatar)
        avatar->setPixmap(QPixmap(":img/contact_dark.svg"));
}

void FriendWidget::setAsInactiveChatroom()
{
    setActive(false);

    if (isDefaultAvatar)
        avatar->setPixmap(QPixmap(":img/contact.svg"));
}

void FriendWidget::updateStatusLight()
{
    Friend* f = FriendList::findFriend(friendId);
    Status status = f->getStatus();

    if (status == Status::Online && !f->getEventFlag())
        statusPic.setPixmap(QPixmap(":img/status/dot_online.svg"));
    else if (status == Status::Online && f->getEventFlag())
        statusPic.setPixmap(QPixmap(":img/status/dot_online_notification.svg"));
    else if (status == Status::Away && !f->getEventFlag())
        statusPic.setPixmap(QPixmap(":img/status/dot_away.svg"));
    else if (status == Status::Away && f->getEventFlag())
        statusPic.setPixmap(QPixmap(":img/status/dot_away_notification.svg"));
    else if (status == Status::Busy && !f->getEventFlag())
        statusPic.setPixmap(QPixmap(":img/status/dot_busy.svg"));
    else if (status == Status::Busy && f->getEventFlag())
        statusPic.setPixmap(QPixmap(":img/status/dot_busy_notification.svg"));
    else if (status == Status::Offline && !f->getEventFlag())
        statusPic.setPixmap(QPixmap(":img/status/dot_offline.svg"));
    else if (status == Status::Offline && f->getEventFlag())
        statusPic.setPixmap(QPixmap(":img/status/dot_offline_notification.svg"));

    if (f->getEventFlag()) {
        CircleWidget* circleWidget = CircleWidget::getFromID(Settings::getInstance().getFriendCircleID(
            FriendList::findFriend(friendId)->getPublicKey()));
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

    if (f->getEventFlag())
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

void FriendWidget::search(const QString& searchString, bool hide)
{
    searchName(searchString, hide);
    CircleWidget* circleWidget = CircleWidget::getFromID(
        Settings::getInstance().getFriendCircleID(FriendList::findFriend(friendId)->getPublicKey()));
    if (circleWidget != nullptr)
        circleWidget->search(searchString);
}

bool FriendWidget::chatFormIsSet(bool focus) const
{
    Friend* f = FriendList::findFriend(friendId);
    if (focus) {
        ContentDialog::focusFriend(friendId);
    }

    bool exist = ContentDialog::existsFriendWidget(friendId);
    return exist || f->getChatForm()->isVisible();
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
}

void FriendWidget::onAvatarRemoved(int FriendId)
{
    if (FriendId != friendId)
        return;

    isDefaultAvatar = true;

    if (isActive())
        avatar->setPixmap(QPixmap(":/img/contact_dark.svg"));
    else
        avatar->setPixmap(QPixmap(":/img/contact.svg"));
}

void FriendWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton)
        dragStartPos = ev->pos();

    GenericChatroomWidget::mousePressEvent(ev);
}

void FriendWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (!(ev->buttons() & Qt::LeftButton))
        return;

    if ((dragStartPos - ev->pos()).manhattanLength() > QApplication::startDragDistance()) {
        QMimeData* mdata = new QMimeData;
        mdata->setText(getFriend()->getPublicKey().toString());

        QDrag* drag = new QDrag(this);
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
    Settings::getInstance().setFriendAlias(f->getPublicKey(), alias);
    Settings::getInstance().savePersonal();
}
