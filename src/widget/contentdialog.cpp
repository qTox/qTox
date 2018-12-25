/*
    Copyright © 2015-2018 by The qTox Project Contributors

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
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QGuiApplication>
#include <QMimeData>
#include <QShortcut>
#include <QSplitter>

#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/friendwidget.h"
#include "src/widget/groupwidget.h"
#include "src/widget/form/chatform.h"
#include "src/widget/friendlistlayout.h"
#include "src/widget/style.h"
#include "src/widget/tool/adjustingscrollarea.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

static const int minWidget = 220;
static const int minHeight = 220;
static const QSize minSize(minHeight, minWidget);
static const QSize defaultSize(720, 400);

ContentDialog::ContentDialog(QWidget* parent)
    : ActivateDialog(parent, Qt::Window)
    , splitter{new QSplitter(this)}
    , friendLayout{new FriendListLayout(this)}
    , activeChatroomWidget(nullptr)
    , videoSurfaceSize(QSize())
    , videoCount(0)
{
    const Settings& s = Settings::getInstance();
    setStyleSheet(Style::getStylesheet("contentDialog/contentDialog.css"));

    friendLayout->setMargin(0);
    friendLayout->setSpacing(0);

    layouts = {friendLayout->getLayoutOnline(), groupLayout.getLayout(),
               friendLayout->getLayoutOffline()};

    if (s.getGroupchatPosition()) {
        layouts.swap(0, 1);
    }

    QWidget* friendWidget = new QWidget();
    friendWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    friendWidget->setAutoFillBackground(true);
    friendWidget->setLayout(friendLayout);

    onGroupchatPositionChanged(s.getGroupchatPosition());

    QScrollArea* friendScroll = new QScrollArea(this);
    friendScroll->setMinimumWidth(minWidget);
    friendScroll->setFrameStyle(QFrame::NoFrame);
    friendScroll->setLayoutDirection(Qt::RightToLeft);
    friendScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    friendScroll->setStyleSheet(Style::getStylesheet("friendList/friendList.css"));
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

    setMinimumSize(minSize);
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("detached");

    QByteArray geometry = s.getDialogGeometry();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    } else {
        resize(defaultSize);
    }

    SplitterRestorer restorer(splitter);
    restorer.restore(s.getDialogSplitterState(), size());

    username = Core::getInstance()->getUsername();

    setAcceptDrops(true);

    new QShortcut(Qt::CTRL + Qt::Key_Q, this, SLOT(close()));
    new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_Tab, this, SLOT(nextContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageUp, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageDown, this, SLOT(nextContact()));

    connect(&s, &Settings::groupchatPositionChanged, this, &ContentDialog::onGroupchatPositionChanged);
    connect(splitter, &QSplitter::splitterMoved, this, &ContentDialog::saveSplitterState);

    Translator::registerHandler(std::bind(&ContentDialog::retranslateUi, this), this);
}

ContentDialog::~ContentDialog()
{
    Translator::unregister(this);
}

void ContentDialog::closeEvent(QCloseEvent *event)
{
    emit willClose();
    event->accept();
}

FriendWidget* ContentDialog::addFriend(std::shared_ptr<FriendChatroom> chatroom, GenericChatForm* form)
{
    const auto compact = Settings::getInstance().getCompactLayout();
    auto frnd = chatroom->getFriend();
    auto friendId = frnd->getId();
    auto friendWidget = new FriendWidget(chatroom, compact);
    friendWidgets[friendId] = friendWidget;
    friendLayout->addFriendWidget(friendWidget, frnd->getStatus());
    friendChatForms[friendId] = form;

    // TODO(sudden6): move this connection to the Friend::displayedNameChanged signal
    connect(frnd, &Friend::aliasChanged, this, &ContentDialog::updateFriendWidget);
    connect(frnd, &Friend::statusMessageChanged, this, &ContentDialog::setStatusMessage);
    connect(friendWidget, &FriendWidget::chatroomWidgetClicked, this, &ContentDialog::activate);

    // FIXME: emit should be removed
    emit friendWidget->chatroomWidgetClicked(friendWidget);

    return friendWidget;
}

GroupWidget* ContentDialog::addGroup(std::shared_ptr<GroupChatroom> chatroom, GenericChatForm* form)
{
    const auto g = chatroom->getGroup();
    const auto groupId = g->getId();
    const auto compact = Settings::getInstance().getCompactLayout();
    auto groupWidget = new GroupWidget(chatroom, compact);
    groupWidgets[groupId] = groupWidget;
    groupLayout.addSortedWidget(groupWidget);
    groupChatForms[groupId] = form;

    connect(groupWidget, &GroupWidget::chatroomWidgetClicked, this, &ContentDialog::activate);

    // FIXME: emit should be removed
    emit groupWidget->chatroomWidgetClicked(groupWidget);

    return groupWidget;
}

void ContentDialog::removeFriend(int friendId)
{
    auto chatroomWidget = qobject_cast<FriendWidget*>(friendWidgets[friendId]);
    disconnect(chatroomWidget->getFriend(), &Friend::aliasChanged, this,
               &ContentDialog::updateFriendWidget);

    // Need to find replacement to show here instead.
    if (activeChatroomWidget == chatroomWidget) {
        cycleContacts(/* forward = */ true, /* inverse = */ false);
    }

    friendLayout->removeFriendWidget(chatroomWidget, Status::Offline);
    friendLayout->removeFriendWidget(chatroomWidget, Status::Online);

    chatroomWidget->deleteLater();

    if (chatroomWidgetCount() == 0) {
        contentLayout->clear();
        activeChatroomWidget = nullptr;
        deleteLater();
    } else {
        update();
    }

    friendWidgets.remove(friendId);
    friendChatForms.remove(friendId);
    closeIfEmpty();
}

void ContentDialog::removeGroup(int groupId)
{
    auto chatroomWidget = qobject_cast<GroupWidget*>(groupWidgets[groupId]);
    // Need to find replacement to show here instead.
    if (activeChatroomWidget == chatroomWidget) {
        cycleContacts(true, false);
    }

    groupLayout.removeSortedWidget(chatroomWidget);
    chatroomWidget->deleteLater();

    if (chatroomWidgetCount() == 0) {
        contentLayout->clear();
        activeChatroomWidget = nullptr;
        deleteLater();
    } else {
        update();
    }

    groupWidgets.remove(groupId);
    groupChatForms.remove(groupId);
    closeIfEmpty();
}

void ContentDialog::closeIfEmpty()
{
    if (friendWidgets.isEmpty() && groupWidgets.isEmpty()) {
        close();
    }
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

/**
 * @brief Get current layout and index of current wiget in it.
 * Current layout -- layout contains activated widget.
 *
 * @param[out] layout Current layout
 * @return Index of current widget in current layout.
 */
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

/**
 * @brief Activate next/previous contact.
 * @param forward If true, activate next contace, previous otherwise.
 * @param inverse ??? TODO: Add docs.
 */
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
        bool nextIsEmpty = (isCurOnline && offlineEmpty && (groupsEmpty || groupsOnTop))
                           || (isCurGroup && offlineEmpty && (onlineEmpty || !groupsOnTop))
                           || (isCurOffline);

        if (nextIsEmpty) {
            forward = !forward;
        }
    }

    index += forward ? 1 : -1;
    // If goes out of the layout, then go to the next and skip empty. This loop goes more
    // then 1 time, because for empty layout index will be out of interval (0 < 0 || 0 >= 0)
    while (index < 0 || index >= currentLayout->count()) {
        int oldCount = currentLayout->count();
        currentLayout = nextLayout(currentLayout, forward);
        int newCount = currentLayout->count();
        if (index < 0) {
            index = newCount - 1;
        } else if (index >= oldCount) {
            index = 0;
        }
    }

    QWidget* widget = currentLayout->itemAt(index)->widget();
    GenericChatroomWidget* chatWidget = qobject_cast<GenericChatroomWidget*>(widget);
    if (chatWidget && chatWidget != activeChatroomWidget) {
        // FIXME: emit should be removed
        emit chatWidget->chatroomWidgetClicked(chatWidget);
    }
}

void ContentDialog::onVideoShow(QSize size)
{
    ++videoCount;
    if (videoCount > 1) {
        return;
    }

    videoSurfaceSize = size;
    QSize minSize = minimumSize();
    setMinimumSize(minSize + videoSurfaceSize);
}

void ContentDialog::onVideoHide()
{
    videoCount--;
    if (videoCount > 0) {
        return;
    }

    QSize minSize = minimumSize();
    setMinimumSize(minSize - videoSurfaceSize);
    videoSurfaceSize = QSize();
}


/**
 * @brief Update window title and icon.
 */
void ContentDialog::updateTitleAndStatusIcon()
{
    if (!activeChatroomWidget) {
        setWindowTitle(username);
        return;
    }

    setWindowTitle(activeChatroomWidget->getTitle() + QStringLiteral(" - ") + username);

    bool isGroupchat = activeChatroomWidget->getGroup() != nullptr;
    if (isGroupchat) {
        setWindowIcon(QIcon(":/img/group.svg"));
        return;
    }

    Status currentStatus = activeChatroomWidget->getFriend()->getStatus();

    QMap<Status, QIcon> icons{{Status::Online, QIcon(":/img/status/online.svg")},
                              {Status::Away, QIcon(":/img/status/away.svg")},
                              {Status::Busy, QIcon(":/img/status/busy.svg")},
                              {Status::Offline, QIcon(":/img/status/offline.svg")}};

    setWindowIcon(icons[currentStatus]);
}

/**
 * @brief Update layouts order according to settings.
 * @param groupOnTop If true move groupchat layout on the top. Move under online otherwise.
 */
void ContentDialog::reorderLayouts(bool newGroupOnTop)
{
    bool oldGroupOnTop = layouts.first() == groupLayout.getLayout();
    if (newGroupOnTop != oldGroupOnTop) {
        layouts.swap(0, 1);
    }
}

void ContentDialog::previousContact()
{
    cycleContacts(false);
}

/**
 * @brief Enable next contact.
 */
void ContentDialog::nextContact()
{
    cycleContacts(true);
}

/**
 * @brief Update username to show in the title.
 * @param newName New name to display.
 */
void ContentDialog::setUsername(const QString& newName)
{
    username = newName;
    updateTitleAndStatusIcon();
}

bool ContentDialog::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::WindowActivate:
        if (activeChatroomWidget) {
            activeChatroomWidget->resetEventFlags();
            activeChatroomWidget->updateStatusLight();

            updateTitleAndStatusIcon();

            const Friend* frnd = activeChatroomWidget->getFriend();
            Group* group = activeChatroomWidget->getGroup();

            if (frnd) {
                emit friendDialogShown(frnd);
            } else if (group) {
                emit groupDialogShown(group);
            }
        }

        emit activated();
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

        int friendId = contact->getId();

        // If friend is already in a dialog then you can't drop friend where it already is.
        if (!hasFriendWidget(friendId)) {
            event->acceptProposedAction();
        }
    } else if (group) {
        qDebug() << event->mimeData()->formats();
        if (!event->mimeData()->hasFormat("groupId")) {
            return;
        }

        int groupId = event->mimeData()->data("groupId").toInt();
        Group* contact = GroupList::findGroup(groupId);
        if (!contact) {
            return;
        }

        if (!hasGroupWidget(groupId)) {
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

        Widget::getInstance()->addGroupDialog(contact, this);
        ensureSplitterVisible();
    }
}

void ContentDialog::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            emit activated();
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
    // Ignore escape keyboard shortcut.
    if (event->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(event);
    }
}

void ContentDialog::focusFriend(int friendId)
{
    focusCommon(friendId, friendWidgets);
}

void ContentDialog::focusGroup(int groupId)
{
    focusCommon(groupId, groupWidgets);
}

void ContentDialog::focusCommon(int id, QHash<int, GenericChatroomWidget*> list)
{
    auto it = list.find(id);
    if (it == list.end()) {
        return;
    }

    activate(*it);
}

/**
 * @brief Show ContentDialog, activate chatroom widget.
 * @param widget Widget which should be activated.
 */
void ContentDialog::activate(GenericChatroomWidget* widget)
{
    // If we clicked on the currently active widget, don't reload and relayout everything
    if (activeChatroomWidget == widget) {
        return;
    }

    contentLayout->clear();

    if (activeChatroomWidget) {
        activeChatroomWidget->setAsInactiveChatroom();
    }

    activeChatroomWidget = widget;

    const FriendWidget* const friendWidget = qobject_cast<FriendWidget*>(widget);
    if (friendWidget) {
        auto friendId = friendWidget->getFriend()->getId();
        friendChatForms[friendId]->show(contentLayout);
    } else {
        auto groupId = widget->getGroup()->getId();
        groupChatForms[groupId]->show(contentLayout);
    }

    widget->setAsActiveChatroom();
    widget->resetEventFlags();
    widget->updateStatusLight();
    updateTitleAndStatusIcon();
}

bool ContentDialog::containsFriend(int friendId) const
{
    return friendWidgets.contains(friendId);
}

bool ContentDialog::containsGroup(int groupId) const
{
    return groupWidgets.contains(groupId);
}

void ContentDialog::updateFriendStatus(int friendId, Status status)
{
    auto widget = qobject_cast<FriendWidget*>(friendWidgets.value(friendId));
    addFriendWidget(widget, status);
}

void ContentDialog::updateFriendStatusLight(int friendId)
{
    auto widget = friendWidgets.value(friendId);
    if (widget != nullptr) {
        widget->updateStatusLight();
    }
}

void ContentDialog::updateGroupStatusLight(int groupId)
{
    auto widget = groupWidgets.value(groupId);
    if (widget != nullptr) {
        widget->updateStatusLight();
    }
}

bool ContentDialog::isFriendWidgetActive(int friendId)
{
    auto widget = friendWidgets.value(friendId);
    if (widget == nullptr) {
        return false;
    }

    return widget->isActive();
}

bool ContentDialog::isGroupWidgetActive(int groupId)
{
    auto widget = friendWidgets.value(groupId);
    if (widget == nullptr) {
        return false;
    }

    return widget->isActive();
}

// TODO: Connect to widget directly
void ContentDialog::setStatusMessage(int friendId, const QString& message)
{
    auto widget = friendWidgets.value(friendId);
    if (widget != nullptr) {
        widget->setStatusMsg(message);
    }
}

/**
 * @brief Update friend widget name and position.
 * @param friendId Friend Id.
 * @param alias Alias to display on widget.
 */
void ContentDialog::updateFriendWidget(uint32_t friendId, QString alias)
{
    Friend* f = FriendList::findFriend(friendId);
    FriendWidget* friendWidget = qobject_cast<FriendWidget*>(friendWidgets[friendId]);

    Status status = f->getStatus();
    friendLayout->addFriendWidget(friendWidget, status);
}

/**
 * @brief Handler of `groupchatPositionChanged` action.
 * Move group layout on the top or on the buttom.
 *
 * @param top If true, move group layout on the top, false otherwise.
 */
void ContentDialog::onGroupchatPositionChanged(bool top)
{
    friendLayout->removeItem(groupLayout.getLayout());
    friendLayout->insertLayout(top ? 0 : 1, groupLayout.getLayout());
}

/**
 * @brief Retranslate all elements in the form.
 */
void ContentDialog::retranslateUi()
{
    updateTitleAndStatusIcon();
}

/**
 * @brief Save size of dialog window.
 */
void ContentDialog::saveDialogGeometry()
{
    Settings::getInstance().setDialogGeometry(saveGeometry());
}

/**
 * @brief Save state of splitter between dialog and dialog list.
 */
void ContentDialog::saveSplitterState()
{
    Settings::getInstance().setDialogSplitterState(splitter->saveState());
}

bool ContentDialog::hasFriendWidget(int friendId) const
{
    return friendWidgets.contains(friendId);
}

bool ContentDialog::hasGroupWidget(int groupId) const
{
    return groupWidgets.contains(groupId);
}

/**
 * @brief Find the next or previous layout in layout list.
 * @param layout Current layout.
 * @param forward If true, move forward, backward othwerwise.
 * @return Next/previous layout.
 */
QLayout* ContentDialog::nextLayout(QLayout* layout, bool forward) const
{
    int index = layouts.indexOf(layout);
    if (index == -1) {
        return nullptr;
    }

    int next = forward ? index + 1 : index - 1;
    size_t size = layouts.size();
    next = (next + size) % size;

    return layouts[next];
}

void ContentDialog::addFriendWidget(FriendWidget* widget, Status status)
{
    friendLayout->addFriendWidget(widget, status);
}

bool ContentDialog::isActiveWidget(GenericChatroomWidget* widget)
{
    return activeChatroomWidget == widget;
}
