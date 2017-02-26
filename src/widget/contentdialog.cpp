/*
    Copyright © 2015 by The qTox Project Contributors

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

#include "contentdialog.h"
#include "splitterrestorer.h"

#include <QBoxLayout>
#include <QDragEnterEvent>
#include <QGuiApplication>
#include <QMimeData>
#include <QShortcut>
#include <QSplitter>

#include "contentlayout.h"
#include "friendwidget.h"
#include "groupwidget.h"
#include "style.h"
#include "widget.h"
#include "src/core/core.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "src/persistence/settings.h"
#include "src/widget/form/chatform.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/friendlistlayout.h"
#include "src/widget/translator.h"
#include "tool/adjustingscrollarea.h"

ContentDialog* ContentDialog::currentDialog = nullptr;
QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> ContentDialog::friendList;
QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> ContentDialog::groupList;

ContentDialog::ContentDialog(SettingsWidget* settingsWidget, QWidget* parent)
    : ActivateDialog(parent, Qt::Window)
    , splitter{new QSplitter(this)}
    , friendLayout{new FriendListLayout(this)}
    , activeChatroomWidget(nullptr)
    , settingsWidget(settingsWidget)
    , videoSurfaceSize(QSize())
    , videoCount(0)
{
    const Settings& s = Settings::getInstance();
    setStyleSheet(Style::getStylesheet(":/ui/contentDialog/contentDialog.css"));

    friendLayout->setMargin(0);
    friendLayout->setSpacing(0);

    QWidget* friendWidget = new QWidget();
    friendWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    friendWidget->setAutoFillBackground(true);
    friendWidget->setLayout(friendLayout);

    onGroupchatPositionChanged(s.getGroupchatPosition());

    QScrollArea* friendScroll = new QScrollArea(this);
    friendScroll->setMinimumWidth(220);
    friendScroll->setFrameStyle(QFrame::NoFrame);
    friendScroll->setLayoutDirection(Qt::RightToLeft);
    friendScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    friendScroll->setStyleSheet(Style::getStylesheet(":/ui/friendList/friendList.css"));
    friendScroll->setWidgetResizable(true);
    friendScroll->setWidget(friendWidget);

    QWidget* contentWidget = new QWidget(this);
    contentWidget->setAutoFillBackground(true);

    contentLayout = new ContentLayout(contentWidget);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);

    splitter->addWidget(friendScroll);
    splitter->addWidget(contentWidget);
    splitter->setStretchFactor(1, 1);
    splitter->setCollapsible(1, false);

    QVBoxLayout* boxLayout = new QVBoxLayout(this);
    boxLayout->setMargin(0);
    boxLayout->setSpacing(0);
    boxLayout->addWidget(splitter);

    setMinimumSize(500, 220);
    setAttribute(Qt::WA_DeleteOnClose);

    QByteArray geometry = s.getDialogGeometry();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    } else {
        resize(720, 400);
    }

    SplitterRestorer restorer(splitter);
    restorer.restore(s.getDialogSettingsGeometry(), size());

    currentDialog = this;
    setAcceptDrops(true);

    new QShortcut(Qt::CTRL + Qt::Key_Q, this, SLOT(close()));
    new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_Tab, this, SLOT(nextContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageUp, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageDown, this, SLOT(nextContact()));

    connect(&s, &Settings::groupchatPositionChanged, this, &ContentDialog::onGroupchatPositionChanged);
    connect(splitter, &QSplitter::splitterMoved, this, &ContentDialog::saveSplitterState);
    connect(Core::getInstance(), &Core::usernameSet, this, &ContentDialog::updateTitleAndStatusIcon);

    Translator::registerHandler(std::bind(&ContentDialog::retranslateUi, this), this);
}

void ContentDialog::removeCurrent(QHash<int, ContactInfo> infos) {
    for (auto it = infos.begin(); it != infos.end(); ) {
        if (std::get<0>(*it) == this) {
            it = infos.erase(it);
        } else {
            ++it;
        }
    }
}

ContentDialog::~ContentDialog()
{
    if (currentDialog == this) {
        currentDialog = nullptr;
    }

    removeCurrent(friendList);
    removeCurrent(groupList);

    Translator::unregister(this);
}

FriendWidget* ContentDialog::addFriend(int friendId, QString id)
{
    FriendWidget* friendWidget = new FriendWidget(friendId, id);
    Friend* frnd = friendWidget->getFriend();
    friendLayout->addFriendWidget(friendWidget, frnd->getStatus());

    connect(frnd, &Friend::aliasChanged, this, &ContentDialog::updateFriendWidget);
    connect(friendWidget, &FriendWidget::chatroomWidgetClicked, this,
            &ContentDialog::onChatroomWidgetClicked);
    connect(friendWidget, SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)),
            frnd->getChatForm(), SLOT(focusInput()));
    connect(Core::getInstance(), &Core::friendAvatarChanged, friendWidget,
            &FriendWidget::onAvatarChange);
    connect(Core::getInstance(), &Core::friendAvatarRemoved, friendWidget,
            &FriendWidget::onAvatarRemoved);

    ContentDialog* lastDialog = getFriendDialog(friendId);
    if (lastDialog) {
        lastDialog->removeFriend(friendId);
    }

    friendList.insert(friendId, std::make_tuple(this, friendWidget));
    // FIXME: emit should be removed
    emit friendWidget->chatroomWidgetClicked(friendWidget, false);

    return friendWidget;
}

GroupWidget* ContentDialog::addGroup(int groupId, const QString& name)
{
    GroupWidget* groupWidget = new GroupWidget(groupId, name);
    groupLayout.addSortedWidget(groupWidget);

    Group* group = groupWidget->getGroup();
    connect(group, &Group::titleChanged, this, &ContentDialog::updateGroupWidget);
    connect(group, &Group::userListChanged, this, &ContentDialog::updateGroupWidget);
    connect(groupWidget, &GroupWidget::chatroomWidgetClicked, this,
            &ContentDialog::onChatroomWidgetClicked);

    ContentDialog* lastDialog = getGroupDialog(groupId);

    if (lastDialog) {
        lastDialog->removeGroup(groupId);
    }

    groupList.insert(groupId, std::make_tuple(this, groupWidget));
    // FIXME: emit should be removed
    emit groupWidget->chatroomWidgetClicked(groupWidget, false);

    return groupWidget;
}

void ContentDialog::removeFriend(int friendId)
{
    auto iter = friendList.find(friendId);
    if (iter == friendList.end()) {
        return;
    }

    FriendWidget* chatroomWidget = static_cast<FriendWidget*>(std::get<1>(iter.value()));
    disconnect(chatroomWidget->getFriend(), &Friend::aliasChanged, this,
               &ContentDialog::updateFriendWidget);

    // Need to find replacement to show here instead.
    if (activeChatroomWidget == chatroomWidget) {
        cycleContacts(true, false);
    }

    friendLayout->removeFriendWidget(chatroomWidget, Status::Offline);
    friendLayout->removeFriendWidget(chatroomWidget, Status::Online);

    chatroomWidget->deleteLater();
    friendList.remove(friendId);

    if (chatroomWidgetCount() == 0) {
        contentLayout->clear();
        activeChatroomWidget = nullptr;
        deleteLater();
    } else {
        update();
    }
}

void ContentDialog::removeGroup(int groupId)
{
    Group* group = GroupList::findGroup(groupId);

    if (group) {
        disconnect(group, &Group::titleChanged, this, &ContentDialog::updateGroupWidget);
        disconnect(group, &Group::userListChanged, this, &ContentDialog::updateGroupWidget);
    }

    auto iter = groupList.find(groupId);
    if (iter == groupList.end()) {
        return;
    }

    GenericChatroomWidget* chatroomWidget = std::get<1>(iter.value());

    // Need to find replacement to show here instead.
    if (activeChatroomWidget == chatroomWidget) {
        cycleContacts(true, false);
    }

    groupLayout.removeSortedWidget(chatroomWidget);
    chatroomWidget->deleteLater();
    groupList.remove(groupId);

    if (chatroomWidgetCount() == 0) {
        contentLayout->clear();
        activeChatroomWidget = nullptr;
        deleteLater();
    } else {
        update();
    }
}

bool ContentDialog::hasFriendWidget(int friendId, GenericChatroomWidget* chatroomWidget)
{
    return hasWidget(friendId, chatroomWidget, friendList);
}

bool ContentDialog::hasGroupWidget(int groupId, GenericChatroomWidget* chatroomWidget)
{
    return hasWidget(groupId, chatroomWidget, groupList);
}

int ContentDialog::chatroomWidgetCount() const
{
    return friendLayout->friendTotalCount() + groupLayout.getLayout()->count();
}

void ContentDialog::ensureSplitterVisible()
{
    if (splitter->sizes().at(0) == 0) {
        splitter->setSizes({1, 1});
    }

    update();
}

int ContentDialog::getCurrentLayout(QLayout*& layout)
{
    layout = friendLayout->getLayoutOnline();
    int index = friendLayout->indexOfFriendWidget(activeChatroomWidget, true);
    if (index != -1) {
        return index;
    }

    layout = friendLayout->getLayoutOffline();
    index = friendLayout->indexOfFriendWidget(activeChatroomWidget, false);
    if (index != -1) {
        return index;
    }

    layout = groupLayout.getLayout();
    index = groupLayout.indexOfSortedWidget(activeChatroomWidget);
    if (index != -1) {
        return index;
    }

    layout = nullptr;
    return -1;
}

void ContentDialog::cycleContacts(bool forward, bool inverse)
{
    QLayout* currentLayout;
    int index = getCurrentLayout(currentLayout);
    if (!currentLayout || index == -1) {
        return;
    }

    if (!inverse && index == currentLayout->count() - 1) {
        bool groupsOnTop = Settings::getInstance().getGroupchatPosition();
        bool offlineEmpty = friendLayout->getLayoutOffline()->isEmpty();
        bool onlineEmpty = friendLayout->getLayoutOnline()->isEmpty();
        bool groupsEmpty = groupLayout.getLayout()->isEmpty();
        bool isCurOffline = currentLayout == friendLayout->getLayoutOffline();
        bool isCurOnline = currentLayout == friendLayout->getLayoutOnline();
        bool isCurGroup = currentLayout == groupLayout.getLayout();
        bool nextIsEmpty = isCurOnline && offlineEmpty && (groupsEmpty || groupsOnTop) ||
                           isCurGroup  && offlineEmpty && (onlineEmpty || !groupsOnTop) ||
                           isCurOffline;

        if (nextIsEmpty) {
            forward = !forward;
        }
    }

    index += forward ? 1 : -1;
    // If goes out of the layout, then go to the next and skip empty
    while (!(0 <= index && index < currentLayout->count())) {
        currentLayout = nextLayout(currentLayout, forward);
        int count = currentLayout->count();
        if (index < 0) {
            index = count - 1;
        } else if (index >= count) {
            index = 0;
        }
    }

    QWidget* widget = currentLayout->itemAt(index)->widget();
    GenericChatroomWidget* chatWidget = qobject_cast<GenericChatroomWidget*>(widget);
    if (chatWidget && chatWidget != activeChatroomWidget) {
        // FIXME: emit should be removed
        emit chatWidget->chatroomWidgetClicked(chatWidget, false);
    }
}

void ContentDialog::onVideoShow(QSize size)
{
    ++videoCount;
    if (videoCount > 1)
        return;

    videoSurfaceSize = size;
    QSize minSize = minimumSize();
    setMinimumSize(minSize + videoSurfaceSize);
}

void ContentDialog::onVideoHide()
{
    videoCount--;
    if (videoCount > 0)
        return;

    QSize minSize = minimumSize();
    setMinimumSize(minSize - videoSurfaceSize);
    videoSurfaceSize = QSize();
}

ContentDialog* ContentDialog::current()
{
    return currentDialog;
}

bool ContentDialog::existsFriendWidget(int friendId, bool focus)
{
    return existsWidget(friendId, focus, friendList);
}

bool ContentDialog::existsGroupWidget(int groupId, bool focus)
{
    return existsWidget(groupId, focus, groupList);
}

void ContentDialog::updateFriendStatus(int friendId)
{
    updateStatus(friendId, friendList);
    ContentDialog* contentDialog = getFriendDialog(friendId);
    if (contentDialog) {
        auto iter = friendList.find(friendId).value();
        GenericChatroomWidget* widget = std::get<1>(iter);
        FriendWidget* friendWidget = static_cast<FriendWidget*>(widget);

        Friend* f = FriendList::findFriend(friendId);
        contentDialog->friendLayout->addFriendWidget(friendWidget, f->getStatus());
    }
}

void ContentDialog::updateFriendStatusMessage(int friendId, const QString& message)
{
    auto iter = friendList.find(friendId);

    if (iter == friendList.end()) {
        return;
    }

    std::get<1>(iter.value())->setStatusMsg(message);
}

void ContentDialog::updateGroupStatus(int groupId)
{
    updateStatus(groupId, groupList);
}

bool ContentDialog::isFriendWidgetActive(int friendId)
{
    return isWidgetActive(friendId, friendList);
}

bool ContentDialog::isGroupWidgetActive(int groupId)
{
    return isWidgetActive(groupId, groupList);
}

ContentDialog* ContentDialog::getFriendDialog(int friendId)
{
    return getDialog(friendId, friendList);
}

ContentDialog* ContentDialog::getGroupDialog(int groupId)
{
    return getDialog(groupId, groupList);
}

void ContentDialog::updateTitleAndStatusIcon(const QString& username)
{
    if (!displayWidget) {
        setWindowTitle(username);
        return;
    }

    setWindowTitle(displayWidget->getTitle() + QStringLiteral(" - ") + username);

    // it's null when it's a groupchat
    if (!displayWidget->getFriend()) {
        setWindowIcon(QIcon(":/img/group.svg"));
        return;
    }

    Status currentStatus = displayWidget->getFriend()->getStatus();

    QMap<Status, QIcon> icons {
        {Status::Online,  QIcon(":/img/status/dot_online.svg")},
        {Status::Away,    QIcon(":/img/status/dot_away.svg")},
        {Status::Busy,    QIcon(":/img/status/dot_busy.svg")},
        {Status::Offline, QIcon(":/img/status/dot_offline.svg")}
    };

    setWindowIcon(icons[currentStatus]);
}

void ContentDialog::updateTitle(GenericChatroomWidget* chatroomWidget)
{
    displayWidget = chatroomWidget;
    updateTitleAndStatusIcon(Core::getInstance()->getUsername());
}

void ContentDialog::previousContact()
{
    cycleContacts(false);
}

void ContentDialog::nextContact()
{
    cycleContacts(true);
}

bool ContentDialog::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::WindowActivate:
        if (activeChatroomWidget) {
            activeChatroomWidget->resetEventFlags();
            activeChatroomWidget->updateStatusLight();
            updateTitle(activeChatroomWidget);

            Friend* frnd = activeChatroomWidget->getFriend();
            Group* group = activeChatroomWidget->getGroup();

            if (frnd) {
                emit friendDialogShown(frnd);
            } else if (group) {
                emit groupDialogShown(group);
            }
        }

        currentDialog = this;

#ifdef Q_OS_MAC
        emit activated();
#endif
    default:
        break;
    }

    return ActivateDialog::event(event);
}

void ContentDialog::dragEnterEvent(QDragEnterEvent* event)
{
    QObject* o = event->source();
    FriendWidget* frnd = qobject_cast<FriendWidget*>(o);
    GroupWidget* group = qobject_cast<GroupWidget*>(o);
    if (frnd) {
        ToxId toxId(event->mimeData()->text());
        Friend* contact = FriendList::findFriend(toxId.getPublicKey());
        if (!contact) {
            return;
        }

        int friendId = contact->getFriendId();
        auto iter = friendList.find(friendId);

        // If friend is already in a dialog then you can't drop friend where it already is.
        if (iter == friendList.end() || std::get<0>(iter.value()) != this) {
            event->acceptProposedAction();
        }
    } else if (group) {
        if (!event->mimeData()->hasFormat("groupId")) {
            return;
        }

        int groupId = event->mimeData()->data("groupId").toInt();
        Group* contact = GroupList::findGroup(groupId);
        if (!contact) {
            return;
        }

        auto iter = groupList.find(groupId);
        if (iter == groupList.end() || std::get<0>(iter.value()) != this) {
            event->acceptProposedAction();
        }
    }
}

void ContentDialog::dropEvent(QDropEvent* event)
{
    QObject* o = event->source();
    FriendWidget* frnd = qobject_cast<FriendWidget*>(o);
    GroupWidget* group = qobject_cast<GroupWidget*>(o);
    if (frnd) {
        ToxId toxId(event->mimeData()->text());
        Friend* contact = FriendList::findFriend(toxId.getPublicKey());
        if (!contact) {
            return;
        }

        int friendId = contact->getFriendId();
        auto iter = friendList.find(friendId);
        if (iter != friendList.end()) {
            std::get<0>(iter.value())->removeFriend(friendId);
        }

        Widget::getInstance()->addFriendDialog(contact, this);
        ensureSplitterVisible();
    } else if (group) {
        if (!event->mimeData()->hasFormat("groupId")) {
            return;
        }

        int groupId = event->mimeData()->data("groupId").toInt();
        Group* contact = GroupList::findGroup(groupId);
        if (!contact) {
            return;
        }

        auto iter = friendList.find(groupId);
        if (iter != friendList.end()) {
            std::get<0>(iter.value())->removeGroup(groupId);
        }

        Widget::getInstance()->addGroupDialog(contact, this);
        ensureSplitterVisible();
    }
}

void ContentDialog::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            currentDialog = this;
        }
    }
}

void ContentDialog::resizeEvent(QResizeEvent* event)
{
    saveDialogGeometry();
    QDialog::resizeEvent(event);
}

void ContentDialog::moveEvent(QMoveEvent* event)
{
    saveDialogGeometry();
    QDialog::moveEvent(event);
}

void ContentDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(event); // Ignore escape keyboard shortcut.
}

void ContentDialog::onChatroomWidgetClicked(GenericChatroomWidget* widget, bool group)
{
    if (group) {
        ContentDialog* contentDialog = new ContentDialog(settingsWidget);
        contentDialog->show();

        if (widget->getFriend()) {
            removeFriend(widget->getFriend()->getFriendId());
            Widget::getInstance()->addFriendDialog(widget->getFriend(), contentDialog);
        } else {
            removeGroup(widget->getGroup()->getGroupId());
            Widget::getInstance()->addGroupDialog(widget->getGroup(), contentDialog);
        }

        contentDialog->raise();
        contentDialog->activateWindow();

        return;
    }

    // If we clicked on the currently active widget, don't reload and relayout everything
    if (activeChatroomWidget == widget) {
        return;
    }

    contentLayout->clear();

    if (activeChatroomWidget) {
        activeChatroomWidget->setAsInactiveChatroom();
    }

    activeChatroomWidget = widget;

    widget->setChatForm(contentLayout);
    widget->setAsActiveChatroom();
    widget->resetEventFlags();
    widget->updateStatusLight();
    updateTitle(widget);
}

void ContentDialog::updateFriendWidget(uint32_t friendId, QString alias)
{
    Friend* f = FriendList::findFriend(friendId);
    GenericChatroomWidget* widget = std::get<1>(friendList.find(friendId).value());
    FriendWidget* friendWidget = static_cast<FriendWidget*>(widget);
    friendWidget->setName(alias);

    Status status = f->getStatus();
    friendLayout->addFriendWidget(friendWidget, status);
}

void ContentDialog::updateGroupWidget(GroupWidget* w)
{
    std::get<1>(groupList.find(w->groupId).value())->setName(w->getName());
    static_cast<GroupWidget*>(std::get<1>(groupList.find(w->groupId).value()))->onUserListChanged();
}

void ContentDialog::onGroupchatPositionChanged(bool top)
{
    friendLayout->removeItem(groupLayout.getLayout());
    friendLayout->insertLayout(top ? 0 : 1, groupLayout.getLayout());
}

void ContentDialog::retranslateUi()
{
    updateTitleAndStatusIcon(Core::getInstance()->getUsername());
}

void ContentDialog::saveDialogGeometry()
{
    Settings::getInstance().setDialogGeometry(saveGeometry());
}

void ContentDialog::saveSplitterState()
{
    Settings::getInstance().setDialogSplitterState(splitter->saveState());
}

bool ContentDialog::hasWidget(int id, GenericChatroomWidget* chatroomWidget,
                              const QHash<int, ContactInfo>& list)
{
    auto iter = list.find(id);

    return iter != list.end() &&
            std::get<0>(*iter) == this &&
            std::get<1>(*iter) == chatroomWidget;
}

bool ContentDialog::existsWidget(int id, bool focus, const QHash<int, ContactInfo>& list)
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return false;
    }

    if (focus)
    {
        ContentDialog* dialog = std::get<0>(*iter);
        if (dialog->windowState() & Qt::WindowMinimized) {
            dialog->showNormal();
        }

        dialog->raise();
        dialog->activateWindow();
        dialog->onChatroomWidgetClicked(std::get<1>(iter.value()), false);
    }

    return true;
}

void ContentDialog::updateStatus(int id, const QHash<int, ContactInfo>& list)
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return;
    }

    GenericChatroomWidget* chatroomWidget = std::get<1>(iter.value());
    chatroomWidget->updateStatusLight();

    if (chatroomWidget->isActive()) {
        std::get<0>(iter.value())->updateTitle(chatroomWidget);
    }
}

bool ContentDialog::isWidgetActive(int id, const QHash<int, ContactInfo>& list)
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return false;
    }

    return std::get<0>(iter.value())->activeChatroomWidget == std::get<1>(iter.value());
}

ContentDialog* ContentDialog::getDialog(int id, const QHash<int, ContactInfo>& list)
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return nullptr;
    }

    return std::get<0>(iter.value());
}

QLayout* ContentDialog::nextLayout(QLayout* layout, bool forward) const
{
    const QVector<QLayout*> layouts = {
        friendLayout->getLayoutOnline(),
        friendLayout->getLayoutOffline(),
        groupLayout.getLayout()
    };

    int index = layouts.indexOf(layout);
    if (index == -1) {
        return nullptr;
    }

    bool groupchatOnTop = Settings::getInstance().getGroupchatPosition();
    int next = forward == groupchatOnTop ? index + 1 : index - 1;
    size_t size = layouts.size();
    next = (next + size) % size;

    return layouts[next];
}
