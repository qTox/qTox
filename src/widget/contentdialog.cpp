/*
    Copyright © 2015 by The qTox Project

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
#include "tool/adjustingscrollarea.h"
#include "src/persistence/settings.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include "src/group.h"
#include "src/grouplist.h"
#include "src/widget/form/chatform.h"
#include "src/core/core.h"
#include "src/widget/friendlistlayout.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/translator.h"

QPointer<ContentDialog> ContentDialog::currentDialog;
QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> ContentDialog::friendList;
QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> ContentDialog::groupList;

ContentDialog::ContentDialog(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f | Qt::Window)
    , splitter(new QSplitter(this))
    , videoSurfaceSize(QSize())
    , videoCount(0)
{
    currentDialog = this;

    const Settings& s = Settings::getInstance();

    setStyleSheet(Style::getStylesheet(":/ui/contentDialog/contentDialog.css"));
    splitter->setHandleWidth(6);

    QWidget *friendWidget = new QWidget(this);
    friendWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    friendLayout = new FriendListLayout(friendWidget);
    friendLayout->setMargin(0);
    friendLayout->setSpacing(0);

    onGroupchatPositionChanged(s.getGroupchatPosition());

    QScrollArea *friendScroll = new QScrollArea(this);
    friendScroll->setMinimumWidth(220);
    friendScroll->setFrameStyle(QFrame::NoFrame);
    friendScroll->setLayoutDirection(Qt::RightToLeft);
    friendScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    friendScroll->setStyleSheet(Style::getStylesheet(":/ui/friendList/friendList.css"));
    friendScroll->setWidgetResizable(true);
    friendScroll->setWidget(friendWidget);

    QWidget* contentWidget = new QWidget(this);
    contentWidget->setAutoFillBackground(true);

    splitter->addWidget(friendScroll);
    splitter->addWidget(contentWidget);
    splitter->setStretchFactor(1, 1);
    splitter->setCollapsible(1, false);

    connect(splitter, &QSplitter::splitterMoved, this, &ContentDialog::saveSplitterState);

    // settings
    connect(&s, &Settings::groupchatPositionChanged,
            this, &ContentDialog::onGroupchatPositionChanged);

    setMinimumSize(500, 220);

    QByteArray geometry = s.getDialogGeometry();

    if (!geometry.isNull())
        restoreGeometry(geometry);
    else
        resize(720, 400);

    QByteArray splitterState = s.getDialogSplitterState();

    if (!splitterState.isNull())
        splitter->restoreState(splitterState);

    setAcceptDrops(true);

    new QShortcut(Qt::CTRL + Qt::Key_Q, this, SLOT(close()));
    new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_Tab, this, SLOT(nextContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageUp, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageDown, this, SLOT(nextContact()));

    connect(Core::getInstance(), &Core::usernameSet, this, &ContentDialog::updateTitleAndStatusIcon);

    Translator::registerHandler(std::bind(&ContentDialog::retranslateUi, this), this);
}

ContentDialog::~ContentDialog()
{
    auto friendIt = friendList.begin();

    while (friendIt != friendList.end())
    {
        if (std::get<0>(friendIt.value()) == this)
        {
            friendIt = friendList.erase(friendIt);
            continue;
        }
        ++friendIt;
    }

    auto groupIt = groupList.begin();

    while (groupIt != groupList.end())
    {
        if (std::get<0>(groupIt.value()) == this)
        {
            groupIt = groupList.erase(groupIt);
            continue;
        }
        ++groupIt;
    }

    Translator::unregister(this);
}

FriendWidget* ContentDialog::addFriend(int friendId, QString id)
{
    FriendWidget* friendWidget = new FriendWidget(friendId, id);
    friendLayout->addFriendWidget(friendWidget, FriendList::findFriend(friendId)->getStatus());

    Friend* frnd = friendWidget->getFriend();
    connect(frnd, &Friend::displayedNameChanged, this, &ContentDialog::updateFriendWidget);
    connect(friendWidget, &FriendWidget::chatroomWidgetClicked, this, &ContentDialog::onChatroomWidgetClicked);
    connect(Core::getInstance(), &Core::friendAvatarChanged, friendWidget, &FriendWidget::onAvatarChange);
    connect(Core::getInstance(), &Core::friendAvatarRemoved, friendWidget, &FriendWidget::onAvatarRemoved);

    ContentDialog* lastDialog = getFriendDialog(friendId);

    if (lastDialog != nullptr)
        lastDialog->removeFriend(friendId);

    friendList.insert(friendId, std::make_tuple(this, friendWidget));
    onChatroomWidgetClicked(friendWidget, false);

    return friendWidget;
}

GroupWidget* ContentDialog::addGroup(int groupId, const QString& name)
{
    GroupWidget* groupWidget = new GroupWidget(groupId, name);
    groupLayout.addSortedWidget(groupWidget);

    Group* group = groupWidget->getGroup();
    connect(group, &Group::titleChanged, this, &ContentDialog::updateGroupWidget);
    connect(group, &Group::userListChanged, this, &ContentDialog::updateGroupWidget);
    connect(groupWidget, &GroupWidget::chatroomWidgetClicked, this, &ContentDialog::onChatroomWidgetClicked);

    ContentDialog* lastDialog = getGroupDialog(groupId);

    if (lastDialog != nullptr)
        lastDialog->removeGroup(groupId);

    groupList.insert(groupId, std::make_tuple(this, groupWidget));
    onChatroomWidgetClicked(groupWidget, false);

    return groupWidget;
}

void ContentDialog::removeFriend(int friendId)
{
    auto iter = friendList.find(friendId);

    if (iter == friendList.end())
        return;

    FriendWidget* chatroomWidget = static_cast<FriendWidget*>(std::get<1>(iter.value()));
    disconnect(chatroomWidget->getFriend(), &Friend::displayedNameChanged, this, &ContentDialog::updateFriendWidget);

    // Need to find replacement to show here instead.
    if (activeChatroomWidget == chatroomWidget)
        cycleContacts(true, false);

    friendLayout->removeFriendWidget(chatroomWidget, Status::Offline);
    friendLayout->removeFriendWidget(chatroomWidget, Status::Online);

    chatroomWidget->deleteLater();
    friendList.remove(friendId);

    if (chatroomWidgetCount() == 0)
    {
        deleteLater();
    }
    else
    {
        update();
    }
}

void ContentDialog::removeGroup(int groupId)
{
    Group* group = GroupList::findGroup(groupId);

    if (group)
    {
        disconnect(group, &Group::titleChanged, this, &ContentDialog::updateGroupWidget);
        disconnect(group, &Group::userListChanged, this, &ContentDialog::updateGroupWidget);
    }

    auto iter = groupList.find(groupId);

    if (iter == groupList.end())
        return;

    GenericChatroomWidget* chatroomWidget = std::get<1>(iter.value());

    // Need to find replacement to show here instead.
    if (activeChatroomWidget == chatroomWidget)
        cycleContacts(true, false);

    groupLayout.removeSortedWidget(chatroomWidget);
    chatroomWidget->deleteLater();
    groupList.remove(groupId);

    if (chatroomWidgetCount() == 0)
    {
        deleteLater();
    }
    else
    {
        update();
    }
}

bool ContentDialog::hasFriendWidget(int friendId, GenericChatroomWidget* chatroomWidget)
{
    return hasWidget(friendId, chatroomWidget, friendList);
}

bool ContentDialog::hasGroupWidget(int groupId, GenericChatroomWidget *chatroomWidget)
{
    return hasWidget(groupId, chatroomWidget, groupList);
}

int ContentDialog::chatroomWidgetCount() const
{
    return friendLayout->friendTotalCount() + groupLayout.getLayout()->count();
}

void ContentDialog::ensureSplitterVisible()
{
    if (splitter->sizes().at(0) == 0)
        splitter->setSizes({1, 1});

    update();
}

void ContentDialog::cycleContacts(bool forward, bool loop)
{
    Settings::getInstance().getGroupchatPosition();

    int index;
    QLayout* currentLayout;
    if (activeChatroomWidget->getFriend())
    {
        currentLayout = friendLayout->getLayoutOnline();
        index = friendLayout->indexOfFriendWidget(activeChatroomWidget, true);
        if (index == -1)
        {
            currentLayout = friendLayout->getLayoutOffline();
            index = friendLayout->indexOfFriendWidget(activeChatroomWidget, false);
        }
    }
    else
    {
        currentLayout = groupLayout.getLayout();
        index = groupLayout.indexOfSortedWidget(activeChatroomWidget);
    }

    if (!loop && index == currentLayout->count() - 1)
    {
        bool groupsOnTop = Settings::getInstance().getGroupchatPosition();
        bool offlineEmpty = friendLayout->getLayoutOffline()->isEmpty();
        bool onlineEmpty = offlineEmpty && (friendLayout->getLayoutOnline()->isEmpty() || !groupsOnTop);
        bool groupsEmpty = offlineEmpty && (groupLayout.getLayout()->isEmpty() || groupsOnTop);

        if ((currentLayout == friendLayout->getLayoutOffline())
            || (currentLayout == friendLayout->getLayoutOnline() && groupsEmpty)
            || (currentLayout == groupLayout.getLayout() && onlineEmpty))
        {
            forward = !forward;
        }
    }

    index += forward ? 1 : -1;

    for (;;)
    {
        // Bounds checking.
        if (index < 0)
        {
            currentLayout = nextLayout(currentLayout, forward);
            index = currentLayout->count() - 1;
            continue;
        }
        else if (index >= currentLayout->count())
        {
            currentLayout = nextLayout(currentLayout, forward);
            index = 0;
            continue;
        }

        GenericChatroomWidget* chatWidget = qobject_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());

        if (chatWidget != nullptr && chatWidget != activeChatroomWidget)
            onChatroomWidgetClicked(chatWidget, false);

        return;
    }
}

void ContentDialog::onVideoShow(QSize size)
{
    videoCount++;
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
    if (contentDialog != nullptr)
    {
        FriendWidget* friendWidget = static_cast<FriendWidget*>(std::get<1>(friendList.find(friendId).value()));
        contentDialog->friendLayout->addFriendWidget(friendWidget, FriendList::findFriend(friendId)->getStatus());
    }
}

void ContentDialog::updateFriendStatusMessage(int friendId, const QString &message)
{
    auto iter = friendList.find(friendId);

    if (iter == friendList.end())
        return;

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
    if (displayWidget != nullptr)
    {

        setWindowTitle(displayWidget->getTitle() + QStringLiteral(" - ") + username);

        // it's null when it's a groupchat
        if (displayWidget->getFriend() == nullptr)
        {
            setWindowIcon(QIcon(":/img/group.svg"));
            return;
        }

        Status currentStatus = displayWidget->getFriend()->getStatus();

        switch(currentStatus) {
            case Status::Online:
                setWindowIcon(QIcon(":/img/status/dot_online.svg"));
                break;
            case Status::Away:
                setWindowIcon(QIcon(":/img/status/dot_away.svg"));
                break;
            case Status::Busy:
                setWindowIcon(QIcon(":/img/status/dot_busy.svg"));
                break;
            case Status::Offline:
                setWindowIcon(QIcon(":/img/status/dot_offline.svg"));
                break;
        }
    }
    else
        setWindowTitle(username);
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
    switch (event->type())
    {
    case QEvent::WindowActivate:
        if (activeChatroomWidget != nullptr)
        {
            activeChatroomWidget->resetEventFlags();
            activeChatroomWidget->updateStatusLight();
            updateTitle(activeChatroomWidget);

            Friend* frnd = activeChatroomWidget->getFriend();
            Group* group = activeChatroomWidget->getGroup();

            GenericChatroomWidget *widget = nullptr;

            if (frnd)
                widget = frnd->getFriendWidget();
            else
                widget = group->getGroupWidget();

            widget->resetEventFlags();
            widget->updateStatusLight();

            Widget::getInstance()->updateScroll(widget);
            Widget::getInstance()->resetIcon();
        }

        currentDialog = this;

#ifdef Q_OS_MAC
        emit activated();
#endif
        break;
    default:
        break;
    }

    return QWidget::event(event);
}

void ContentDialog::dragEnterEvent(QDragEnterEvent *event)
{
    QObject *o = event->source();
    FriendWidget *frnd = qobject_cast<FriendWidget*>(o);
    GroupWidget *group = qobject_cast<GroupWidget*>(o);
    if (frnd)
    {
        ToxId toxId(event->mimeData()->text());
        Friend *contact = FriendList::findFriend(toxId);
        if (!contact)
            return;

        int friendId = contact->getFriendId();
        auto iter = friendList.find(friendId);

        // If friend is already in a dialog then you can't drop friend where it already is.
        if (iter == friendList.end() || std::get<0>(iter.value()) != this)
            event->acceptProposedAction();
    }
    else if (group)
    {
        if (!event->mimeData()->hasFormat("groupId"))
            return;

        int groupId = event->mimeData()->data("groupId").toInt();
        Group *contact = GroupList::findGroup(groupId);
        if (!contact)
            return;

        auto iter = groupList.find(groupId);
        if (iter == groupList.end() || std::get<0>(iter.value()) != this)
            event->acceptProposedAction();
    }
}

void ContentDialog::dropEvent(QDropEvent *event)
{
    QObject *o = event->source();
    FriendWidget *frnd = qobject_cast<FriendWidget*>(o);
    GroupWidget *group = qobject_cast<GroupWidget*>(o);
    if (frnd)
    {
        ToxId toxId(event->mimeData()->text());
        Friend *contact = FriendList::findFriend(toxId);
        if (!contact)
            return;

        int friendId = contact->getFriendId();
        auto iter = friendList.find(friendId);
        if (iter != friendList.end())
            std::get<0>(iter.value())->removeFriend(friendId);

        Widget::getInstance()->addFriendDialog(contact, this);
        ensureSplitterVisible();
    }
    else if (group)
    {
        if (!event->mimeData()->hasFormat("groupId"))
            return;

        int groupId = event->mimeData()->data("groupId").toInt();
        Group *contact = GroupList::findGroup(groupId);
        if (!contact)
            return;

        auto iter = friendList.find(groupId);
        if (iter != friendList.end())
            std::get<0>(iter.value())->removeGroup(groupId);

        Widget::getInstance()->addGroupDialog(contact, this);
        ensureSplitterVisible();
    }
}

void ContentDialog::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange)
    {
        if (isActiveWindow())
            currentDialog = this;
    }
}

void ContentDialog::resizeEvent(QResizeEvent* event)
{
    saveDialogGeometry();
    QWidget::resizeEvent(event);
}

void ContentDialog::moveEvent(QMoveEvent* event)
{
    saveDialogGeometry();
    QWidget::moveEvent(event);
}

void ContentDialog::onChatroomWidgetClicked(GenericChatroomWidget *widget, bool group)
{
    if (group)
    {
        ContentDialog* contentDialog = new ContentDialog(this);
        contentDialog->show();

        if (widget->getFriend())
        {
            removeFriend(widget->getFriend()->getFriendId());
            Widget::getInstance()->addFriendDialog(widget->getFriend(), contentDialog);
        }
        else
        {
            removeGroup(widget->getGroup()->getGroupId());
            Widget::getInstance()->addGroupDialog(widget->getGroup(), contentDialog);
        }

        contentDialog->raise();
        contentDialog->activateWindow();

        return;
    }

    // If we clicked on the currently active widget, don't reload and relayout everything
    if (activeChatroomWidget == widget)
        return;

    if (activeChatroomWidget)
        activeChatroomWidget->setAsInactiveChatroom();

    activeChatroomWidget = widget;

    widget->setChatForm();
    widget->setAsActiveChatroom();
    widget->resetEventFlags();
    widget->updateStatusLight();
    updateTitle(widget);

    if (widget->getFriend())
        widget->getFriend()->getFriendWidget()->updateStatusLight();
    else
        widget->getGroup()->getGroupWidget()->updateStatusLight();
}

void ContentDialog::updateFriendWidget(FriendWidget *w, Status s)
{
    FriendWidget* friendWidget = static_cast<FriendWidget*>(std::get<1>(friendList.find(w->friendId).value()));
    friendWidget->setName(w->getName());
    friendLayout->addFriendWidget(friendWidget, s);
}

void ContentDialog::updateGroupWidget(GroupWidget *w)
{
    std::get<1>(groupList.find(w->groupId).value())->setName(w->getName());
    static_cast<GroupWidget*>(std::get<1>(groupList.find(w->groupId).value()))->onUserListChanged();
}

void ContentDialog::onGroupchatPositionChanged(bool top)
{
    friendLayout->removeItem(groupLayout.getLayout());

    if (top)
        friendLayout->insertLayout(0, groupLayout.getLayout());
    else
        friendLayout->insertLayout(1, groupLayout.getLayout());
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

bool ContentDialog::hasWidget(int id, GenericChatroomWidget* chatroomWidget, const QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>>& list)
{
    auto iter = list.find(id);

    if (iter == list.end() || std::get<0>(iter.value()) != this)
        return false;

    return chatroomWidget == std::get<1>(iter.value());
}

bool ContentDialog::existsWidget(int id, bool focus, const QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>>& list)
{
    auto iter = list.find(id);
    if (iter == list.end())
        return false;

    if (focus)
    {
        if (std::get<0>(iter.value())->windowState() & Qt::WindowMinimized)
            std::get<0>(iter.value())->showNormal();

        std::get<0>(iter.value())->raise();
        std::get<0>(iter.value())->activateWindow();
        std::get<0>(iter.value())->onChatroomWidgetClicked(std::get<1>(iter.value()), false);
    }

    return true;
}

void ContentDialog::updateStatus(int id, const QHash<int, std::tuple<ContentDialog *, GenericChatroomWidget *> > &list)
{
    auto iter = list.find(id);

    if (iter == list.end())
        return;

    GenericChatroomWidget* chatroomWidget = std::get<1>(iter.value());
    chatroomWidget->updateStatusLight();

    if (chatroomWidget->isActive())
        std::get<0>(iter.value())->updateTitle(chatroomWidget);
}

bool ContentDialog::isWidgetActive(int id, const QHash<int, std::tuple<ContentDialog *, GenericChatroomWidget *> > &list)
{
    auto iter = list.find(id);

    if (iter == list.end())
        return false;

    return std::get<0>(iter.value())->activeChatroomWidget == std::get<1>(iter.value());
}

ContentDialog* ContentDialog::getDialog(int id, const QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>>& list)
{
    auto iter = list.find(id);

    if (iter == list.end())
        return nullptr;

    return std::get<0>(iter.value());
}

QLayout* ContentDialog::nextLayout(QLayout* layout, bool forward) const
{
    if (layout == groupLayout.getLayout())
    {
        if (forward)
        {
            if (Settings::getInstance().getGroupchatPosition())
                return friendLayout->getLayoutOnline();

            return friendLayout->getLayoutOffline();
        }
        else
        {
            if (Settings::getInstance().getGroupchatPosition())
                return friendLayout->getLayoutOffline();

            return friendLayout->getLayoutOnline();
        }
    }
    else if (layout == friendLayout->getLayoutOnline())
    {
        if (forward)
        {
            if (Settings::getInstance().getGroupchatPosition())
                return friendLayout->getLayoutOffline();

            return groupLayout.getLayout();
        }
        else
        {
            if (Settings::getInstance().getGroupchatPosition())
                return groupLayout.getLayout();

            return friendLayout->getLayoutOffline();
        }
    }
    else if (layout == friendLayout->getLayoutOffline())
    {
        if (forward)
        {
            if (Settings::getInstance().getGroupchatPosition())
                return groupLayout.getLayout();

            return friendLayout->getLayoutOnline();
        }
        else
        {
            if (Settings::getInstance().getGroupchatPosition())
                return friendLayout->getLayoutOnline();

            return groupLayout.getLayout();
        }
    }
    return nullptr;
}
