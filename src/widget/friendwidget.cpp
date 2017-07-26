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
    , frnd{FriendList::findFriend(friendId)}
    , isDefaultAvatar{true}
    , historyLoaded{false}
{
    avatar->setPixmap(QPixmap(":/img/contact.svg"));
    statusPic.setPixmap(QPixmap(":/img/status/dot_offline.svg"));
    statusPic.setMargin(3);
    nameLabel->setText(frnd->getDisplayedName());
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
    if (!active) {
        setBackgroundRole(QPalette::Highlight);
    }

    installEventFilter(this); // Disable leave event.

    QMenu menu;
    QAction* openChatWindow = nullptr;
    QAction* removeChatWindow = nullptr;

    uint32_t friendId = frnd->getFriendId();
    ContentDialog* contentDialog = ContentDialog::getFriendDialog(friendId);

    bool notAlone = contentDialog != nullptr && contentDialog->chatroomWidgetCount() > 1;
    if (!contentDialog || notAlone) {
        openChatWindow = menu.addAction(tr("Open chat in new window"));
    }

    bool hasFriendWidget = contentDialog->hasFriendWidget(friendId, this);
    if (contentDialog && hasFriendWidget) {
        removeChatWindow = menu.addAction(tr("Remove chat from this window"));
    }

    menu.addSeparator();
    QMenu* inviteMenu = menu.addMenu(tr("Invite to group",
                                        "Menu to invite a friend to a groupchat"));
    inviteMenu->setEnabled(frnd->getStatus() != Status::Offline);
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

    int circleId = Settings::getInstance().getFriendCircleID(frnd->getPublicKey());
    CircleWidget* circleWidget = CircleWidget::getFromID(circleId);

    QWidget* w = circleWidget ? circleWidget : static_cast<QWidget*>(this);
    FriendListWidget* friendList = qobject_cast<FriendListWidget*>(w->parentWidget());

    QMenu* circleMenu = menu.addMenu(tr("Move to circle...",
                                        "Menu to move a friend into a different circle"));

    QAction* newCircleAction = circleMenu->addAction(tr("To new circle"));

    QAction* removeCircleAction = nullptr;
    if (circleId != -1) {
        const QString circleName = Settings::getInstance().getCircleName(circleId);
        removeCircleAction = circleMenu->addAction(tr("Remove from circle '%1'").arg(circleName));
    }

    circleMenu->addSeparator();

    QList<QAction*> circleActionList;
    QMap<QAction*, int> circleActions;

    for (int i = 0; i < Settings::getInstance().getCircleCount(); ++i) {
        if (i == circleId) {
            continue;
        }

        const QString name = Settings::getInstance().getCircleName(i);
        QAction* action = new QAction(tr("Move  to circle \"%1\"").arg(name), circleMenu);
        circleActionList.push_back(action);
        circleActions[circleActionList.back()] = i;
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
    QAction* autoAccept = menu.addAction(tr("Auto accept files from this friend",
                                            "context menu entry"));
    ToxPk id = frnd->getPublicKey();
    QString dir = Settings::getInstance().getAutoAcceptDir(id);
    autoAccept->setCheckable(true);
    autoAccept->setChecked(!dir.isEmpty());
    menu.addSeparator();

    QAction* removeFriendAction = nullptr;

    if (!contentDialog || !contentDialog->hasFriendWidget(friendId, this)) {
        removeFriendAction = menu.addAction(tr("Remove friend",
                                               "Menu to remove the friend from our friendlist"));
    }

    menu.addSeparator();
    QAction* aboutWindow = menu.addAction(tr("Show details"));

    QPoint pos = event->globalPos();
    QAction* selectedItem = menu.exec(pos);

    removeEventFilter(this);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }

    if (!selectedItem) {
        return;
    }

    if (selectedItem == setAlias) {
        nameLabel->editBegin();
    } else if (selectedItem == removeFriendAction) {
        emit removeFriend(friendId);
    } else if (selectedItem == openChatWindow) {
        emit newWindowOpened(this);
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
            dir = QFileDialog::getExistingDirectory(Q_NULLPTR,
                                                    tr("Choose an auto accept directory", "popup title"),
                                                    dir);

            autoAccept->setChecked(true);
            qDebug() << "Setting auto accept dir for" << friendId << "to" << dir;
            Settings::getInstance().setAutoAcceptDir(id, dir);
        }
    } else if (selectedItem == aboutWindow) {
        AboutUser* aboutUser = new AboutUser(id, Widget::getInstance());
        aboutUser->setFriend(frnd);
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
            friendList->moveWidget(this, frnd->getStatus(), true);
        else
            Settings::getInstance().setFriendCircleID(id, -1);

        if (circleWidget) {
            circleWidget->updateStatus();
            Widget::getInstance()->searchCircle(circleWidget);
        }
    } else if (circleActions.contains(selectedItem)) {
        CircleWidget* circle = CircleWidget::getFromID(circleActions[selectedItem]);

        if (circle) {
            circle->addFriendWidget(this, frnd->getStatus());
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

    if (isDefaultAvatar) {
        avatar->setPixmap(QPixmap(":img/contact_dark.svg"));
    }
}

void FriendWidget::setAsInactiveChatroom()
{
    setActive(false);

    if (isDefaultAvatar) {
        avatar->setPixmap(QPixmap(":img/contact.svg"));
    }
}

void FriendWidget::updateStatusLight()
{
    static const QString statuses[] = {
        ":img/status/dot_online.svg",
        ":img/status/dot_online_notification.svg",
        ":img/status/dot_away.svg",
        ":img/status/dot_away_notification.svg",
        ":img/status/dot_busy.svg",
        ":img/status/dot_busy_notification.svg",
        ":img/status/dot_offline.svg",
        ":img/status/dot_offline_notification.svg",
    };

    const bool event = frnd->getEventFlag();
    const int index = static_cast<int>(frnd->getStatus()) + event;
    statusPic.setPixmap(QPixmap(statuses[index]));

    if (event) {
        const Settings& s = Settings::getInstance();
        const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
        CircleWidget* circleWidget = CircleWidget::getFromID(circleId);
        if (circleWidget) {
            circleWidget->setExpanded(true);
        }

        Widget::getInstance()->updateFriendActivity(frnd);
    }

    statusPic.setMargin(event ? 0 : 3);
}

QString FriendWidget::getStatusString() const
{
    const int status = static_cast<int>(frnd->getStatus());
    const bool event = frnd->getEventFlag();

    static const QVector<QString> names = {
        tr("Online"),
        tr("Away"),
        tr("Busy"),
        tr("Offline"),
    };

    return event ? tr("New message") : names.value(status);
}

Friend* FriendWidget::getFriend() const
{
    return frnd;
}

void FriendWidget::search(const QString& searchString, bool hide)
{
    searchName(searchString, hide);
    const Settings& s = Settings::getInstance();
    const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
    CircleWidget* circleWidget = CircleWidget::getFromID(circleId);
    if (circleWidget) {
        circleWidget->search(searchString);
    }
}

void FriendWidget::setChatForm(ContentLayout* contentLayout)
{
    frnd->getChatForm()->show(contentLayout);
}

void FriendWidget::resetEventFlags()
{
    frnd->setEventFlag(false);
}

void FriendWidget::onAvatarChange(int friendId, const QPixmap& pic)
{
    if (friendId != frnd->getFriendId()) {
        return;
    }

    isDefaultAvatar = false;
    avatar->setPixmap(pic);
}

void FriendWidget::onAvatarRemoved(int friendId)
{
    if (friendId != frnd->getFriendId()) {
        return;
    }

    isDefaultAvatar = true;

    const QString path = QString(":/img/contact%1.svg").arg(isActive() ? "" : "_dark");
    avatar->setPixmap(QPixmap(path));
}

void FriendWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        dragStartPos = ev->pos();
    }

    GenericChatroomWidget::mousePressEvent(ev);
}

void FriendWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (!(ev->buttons() & Qt::LeftButton)) {
        return;
    }

    const int distance = (dragStartPos - ev->pos()).manhattanLength();
    if (distance > QApplication::startDragDistance()) {
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
    QString alias = _alias.left(tox_max_name_length());
    frnd->setAlias(alias);
    Settings::getInstance().setFriendAlias(frnd->getPublicKey(), alias);
    Settings::getInstance().savePersonal();
}
