/*
    Copyright © 2019 by The qTox Project Contributors

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
#include "friendlistwidget.h"
#include "groupwidget.h"
#include "maskablepixmapwidget.h"

#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/about/aboutfriend.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include "src/widget/about/aboutfriendform.h"
#include "src/widget/form/chatform.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/widget.h"

#include <QApplication>
#include <QBitmap>
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
FriendWidget::FriendWidget(std::shared_ptr<FriendChatroom> chatroom, bool compact)
    : GenericChatroomWidget(compact)
    , chatroom{chatroom}
    , isDefaultAvatar{true}
{
    avatar->setPixmap(QPixmap(":/img/contact.svg"));
    statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Offline)));
    statusPic.setMargin(3);

    auto frnd = chatroom->getFriend();
    nameLabel->setText(frnd->getDisplayedName());
    // update alias when edited
    connect(nameLabel, &CroppingLabel::editFinished, frnd, &Friend::setAlias);
    // update on changes of the displayed name
    connect(frnd, &Friend::displayedNameChanged, nameLabel, &CroppingLabel::setText);
    connect(frnd, &Friend::displayedNameChanged, this,
            [this](const QString /* &newName */) { emit friendWidgetRenamed(this); });
    connect(chatroom.get(), &FriendChatroom::activeChanged, this, &FriendWidget::setActive);
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

    if (chatroom->possibleToOpenInNewWindow()) {
        const auto openChatWindow = menu.addAction(tr("Open chat in new window"));
        connect(openChatWindow, &QAction::triggered, [=]() { emit newWindowOpened(this); });
    }

    if (chatroom->canBeRemovedFromWindow()) {
        const auto removeChatWindow = menu.addAction(tr("Remove chat from this window"));
        connect(removeChatWindow, &QAction::triggered, this, &FriendWidget::removeChatWindow);
    }

    menu.addSeparator();
    QMenu* inviteMenu =
        menu.addMenu(tr("Invite to group", "Menu to invite a friend to a groupchat"));
    inviteMenu->setEnabled(chatroom->canBeInvited());
    const auto newGroupAction = inviteMenu->addAction(tr("To new group"));
    connect(newGroupAction, &QAction::triggered, chatroom.get(), &FriendChatroom::inviteToNewGroup);
    inviteMenu->addSeparator();

    for (const auto& group : chatroom->getGroups()) {
        const auto groupAction = inviteMenu->addAction(tr("Invite to group '%1'").arg(group.name));
        connect(groupAction, &QAction::triggered, [=]() { chatroom->inviteFriend(group.group); });
    }

    const auto circleId = chatroom->getCircleId();
    auto circleMenu =
        menu.addMenu(tr("Move to circle...", "Menu to move a friend into a different circle"));

    const auto newCircleAction = circleMenu->addAction(tr("To new circle"));
    connect(newCircleAction, &QAction::triggered, this, &FriendWidget::moveToNewCircle);

    if (circleId != -1) {
        const auto circleName = chatroom->getCircleName();
        const auto removeCircleAction =
            circleMenu->addAction(tr("Remove from circle '%1'").arg(circleName));
        connect(removeCircleAction, &QAction::triggered, this, &FriendWidget::removeFromCircle);
    }

    circleMenu->addSeparator();

    for (const auto& circle : chatroom->getOtherCircles()) {
        QAction* action = new QAction(tr("Move to circle \"%1\"").arg(circle.name), circleMenu);
        connect(action, &QAction::triggered, [=]() { moveToCircle(circle.circleId); });
        circleMenu->addAction(action);
    }

    const auto setAlias = menu.addAction(tr("Set alias..."));
    connect(setAlias, &QAction::triggered, nameLabel, &CroppingLabel::editBegin);

    menu.addSeparator();
    auto autoAccept =
        menu.addAction(tr("Auto accept files from this friend", "context menu entry"));
    autoAccept->setCheckable(true);
    autoAccept->setChecked(!chatroom->autoAcceptEnabled());
    connect(autoAccept, &QAction::triggered, this, &FriendWidget::changeAutoAccept);
    menu.addSeparator();

    if (chatroom->friendCanBeRemoved()) {
        const auto friendPk = chatroom->getFriend()->getPublicKey();
        const auto removeAction =
            menu.addAction(tr("Remove friend", "Menu to remove the friend from the friend list"));
        connect(removeAction, &QAction::triggered, this, [=]() { emit removeFriend(friendPk); },
                Qt::QueuedConnection);
    }

    menu.addSeparator();
    const auto aboutWindow = menu.addAction(tr("Show details"));
    connect(aboutWindow, &QAction::triggered, this, &FriendWidget::showDetails);

    const auto pos = event->globalPos();
    menu.exec(pos);

    removeEventFilter(this);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }
}

void FriendWidget::removeChatWindow()
{
    chatroom->removeFriendFromDialogs();
}

namespace {

std::tuple<CircleWidget*, FriendListWidget*> getCircleAndFriendList(const Friend* frnd, FriendWidget* fw)
{
    const auto pk = frnd->getPublicKey();
    const auto circleId = Settings::getInstance().getFriendCircleID(pk);
    auto circleWidget = CircleWidget::getFromID(circleId);
    auto w = circleWidget ? static_cast<QWidget*>(circleWidget) : static_cast<QWidget*>(fw);
    auto friendList = qobject_cast<FriendListWidget*>(w->parentWidget());
    return std::make_tuple(circleWidget, friendList);
}

} // namespace

void FriendWidget::moveToNewCircle()
{
    const auto frnd = chatroom->getFriend();
    CircleWidget* circleWidget;
    FriendListWidget* friendList;
    std::tie(circleWidget, friendList) = getCircleAndFriendList(frnd, this);

    if (circleWidget != nullptr) {
        circleWidget->updateStatus();
    }

    if (friendList != nullptr) {
        friendList->addCircleWidget(this);
    } else {
        const auto pk = frnd->getPublicKey();
        auto& s = Settings::getInstance();
        auto circleId = s.addCircle();
        s.setFriendCircleID(pk, circleId);
    }
}

void FriendWidget::removeFromCircle()
{
    const auto frnd = chatroom->getFriend();
    CircleWidget* circleWidget;
    FriendListWidget* friendList;
    std::tie(circleWidget, friendList) = getCircleAndFriendList(frnd, this);

    if (friendList != nullptr) {
        friendList->moveWidget(this, frnd->getStatus(), true);
    } else {
        const auto pk = frnd->getPublicKey();
        auto& s = Settings::getInstance();
        s.setFriendCircleID(pk, -1);
    }

    if (circleWidget != nullptr) {
        circleWidget->updateStatus();
        emit searchCircle(*circleWidget);
    }
}

void FriendWidget::moveToCircle(int newCircleId)
{
    const auto frnd = chatroom->getFriend();
    const auto pk = frnd->getPublicKey();
    const auto oldCircleId = Settings::getInstance().getFriendCircleID(pk);
    auto& s = Settings::getInstance();
    auto oldCircleWidget = CircleWidget::getFromID(oldCircleId);
    auto newCircleWidget = CircleWidget::getFromID(newCircleId);

    if (newCircleWidget) {
        newCircleWidget->addFriendWidget(this, frnd->getStatus());
        newCircleWidget->setExpanded(true);
        emit searchCircle(*newCircleWidget);
        s.savePersonal();
    } else {
        s.setFriendCircleID(pk, newCircleId);
    }

    if (oldCircleWidget) {
        oldCircleWidget->updateStatus();
        emit searchCircle(*oldCircleWidget);
    }
}

void FriendWidget::changeAutoAccept(bool enable)
{
    if (enable) {
        const auto oldDir = chatroom->getAutoAcceptDir();
        const auto newDir =
            QFileDialog::getExistingDirectory(Q_NULLPTR,
                                              tr("Choose an auto accept directory", "popup title"),
                                              oldDir);
        chatroom->setAutoAcceptDir(newDir);
    } else {
        chatroom->disableAutoAccept();
    }
}
void FriendWidget::showDetails()
{
    const auto frnd = chatroom->getFriend();
    const auto iabout = new AboutFriend(frnd, &Settings::getInstance());
    std::unique_ptr<IAboutFriend> about = std::unique_ptr<IAboutFriend>(iabout);
    const auto aboutUser = new AboutFriendForm(std::move(about), this);
    connect(aboutUser, &AboutFriendForm::histroyRemoved, this, &FriendWidget::friendHistoryRemoved);
    aboutUser->show();
}

void FriendWidget::setAsActiveChatroom()
{
    setActive(true);
}

void FriendWidget::setAsInactiveChatroom()
{
    setActive(false);
}

void FriendWidget::setActive(bool active)
{
    GenericChatroomWidget::setActive(active);
    if (isDefaultAvatar) {
        const auto uri =
            active ? QStringLiteral(":img/contact_dark.svg") : QStringLiteral(":img/contact.svg");
        avatar->setPixmap(QPixmap{uri});
    }
}

void FriendWidget::updateStatusLight()
{
    const auto frnd = chatroom->getFriend();
    const bool event = frnd->getEventFlag();
    statusPic.setPixmap(QPixmap(Status::getIconPath(frnd->getStatus(), event)));

    if (event) {
        const Settings& s = Settings::getInstance();
        const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
        CircleWidget* circleWidget = CircleWidget::getFromID(circleId);
        if (circleWidget) {
            circleWidget->setExpanded(true);
        }

        emit updateFriendActivity(*frnd);
    }

    statusPic.setMargin(event ? 1 : 3);
}

QString FriendWidget::getStatusString() const
{
    const auto frnd = chatroom->getFriend();
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

const Friend* FriendWidget::getFriend() const
{
    return chatroom->getFriend();
}

const Contact* FriendWidget::getContact() const
{
    return getFriend();
}

void FriendWidget::search(const QString& searchString, bool hide)
{
    const auto frnd = chatroom->getFriend();
    searchName(searchString, hide);
    const Settings& s = Settings::getInstance();
    const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
    CircleWidget* circleWidget = CircleWidget::getFromID(circleId);
    if (circleWidget) {
        circleWidget->search(searchString);
    }
}

void FriendWidget::resetEventFlags()
{
    chatroom->resetEventFlags();
}

void FriendWidget::onAvatarSet(const ToxPk& friendPk, const QPixmap& pic)
{
    const auto frnd = chatroom->getFriend();
    if (friendPk != frnd->getPublicKey()) {
        return;
    }

    isDefaultAvatar = false;
    avatar->setPixmap(pic);
}

void FriendWidget::onAvatarRemoved(const ToxPk& friendPk)
{
    const auto frnd = chatroom->getFriend();
    if (friendPk != frnd->getPublicKey()) {
        return;
    }

    isDefaultAvatar = true;

    const QString path = QString(":/img/contact%1.svg").arg(isActive() ? "_dark" : "");
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
        const Friend* frnd = getFriend();
        mdata->setText(frnd->getDisplayedName());
        mdata->setData("toxPk", frnd->getPublicKey().getByteArray());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}
