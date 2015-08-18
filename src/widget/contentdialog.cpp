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
#include <QBoxLayout>
#include <QSplitter>
#include <QGuiApplication>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QShortcut>

ContentDialog* ContentDialog::currentDialog = nullptr;
QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> ContentDialog::friendList;
QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> ContentDialog::groupList;

ContentDialog::ContentDialog(SettingsWidget* settingsWidget, QWidget* parent)
    : ActivateDialog(parent, Qt::Window)
    , activeChatroomWidget(nullptr)
    , settingsWidget(settingsWidget)
{
    QVBoxLayout* boxLayout = new QVBoxLayout(this);
    boxLayout->setMargin(0);
    boxLayout->setSpacing(0);

    splitter = new QSplitter(this);
    setStyleSheet("QSplitter{color: rgb(255, 255, 255);background-color: rgb(255, 255, 255);alternate-background-color: rgb(255, 255, 255);border-color: rgb(255, 255, 255);gridline-color: rgb(255, 255, 255);selection-color: rgb(255, 255, 255);selection-background-color: rgb(255, 255, 255);}QSplitter:handle{color: rgb(255, 255, 255);background-color: rgb(255, 255, 255);}");
    splitter->setHandleWidth(6);

    QWidget *friendWidget = new QWidget();
    friendWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    friendWidget->setAutoFillBackground(true);

    friendLayout = new FriendListLayout();
    friendLayout->setMargin(0);
    friendLayout->setSpacing(0);
    friendWidget->setLayout(friendLayout);

    onGroupchatPositionChanged(Settings::getInstance().getGroupchatPosition());

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
    contentLayout = new ContentLayout(contentWidget);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);

    splitter->addWidget(friendScroll);
    splitter->addWidget(contentWidget);
    splitter->setStretchFactor(1, 1);
    splitter->setCollapsible(1, false);
    boxLayout->addWidget(splitter);

    connect(splitter, &QSplitter::splitterMoved, this, &ContentDialog::saveSplitterState);

    connect(settingsWidget, &SettingsWidget::groupchatPositionToggled, this, &ContentDialog::onGroupchatPositionChanged);

    setMinimumSize(500, 220);
    setAttribute(Qt::WA_DeleteOnClose);

    QByteArray geometry = Settings::getInstance().getDialogGeometry();

    if (!geometry.isNull())
        restoreGeometry(geometry);
    else
        resize(720, 400);


    QByteArray splitterState = Settings::getInstance().getDialogSplitterState();

    if (!splitterState.isNull())
        splitter->restoreState(splitterState);

    currentDialog = this;

    setAcceptDrops(true);

    new QShortcut(Qt::CTRL + Qt::Key_Q, this, SLOT(close()));
    new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_Tab, this, SLOT(nextContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageUp, this, SLOT(previousContact()));
    new QShortcut(Qt::CTRL + Qt::Key_PageDown, this, SLOT(nextContact()));

    connect(Core::getInstance(), &Core::usernameSet, this, &ContentDialog::updateTitleUsername);

    Translator::registerHandler(std::bind(&ContentDialog::retranslateUi, this), this);
}

ContentDialog::~ContentDialog()
{
    if (currentDialog == this)
        currentDialog = nullptr;

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
    connect(settingsWidget, &SettingsWidget::compactToggled, friendWidget, &FriendWidget::compactChange);
    connect(friendWidget, &FriendWidget::chatroomWidgetClicked, this, &ContentDialog::onChatroomWidgetClicked);
    connect(friendWidget, SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), frnd->getChatForm(), SLOT(focusInput()));
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
    connect(settingsWidget, &SettingsWidget::compactToggled, groupWidget, &GroupWidget::compactChange);
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

    if (activeChatroomWidget == chatroomWidget)
    {
        // Need to find replacement to show here instead.
        cycleContacts(true, false);
    }

    friendLayout->removeFriendWidget(chatroomWidget, Status::Offline);
    friendLayout->removeFriendWidget(chatroomWidget, Status::Online);

    chatroomWidget->deleteLater();
    friendList.remove(friendId);

    if (chatroomWidgetCount() == 0)
    {
        contentLayout->clear();
        activeChatroomWidget = nullptr;
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

    if (activeChatroomWidget == chatroomWidget)
    {
        // Need to find replacement to show here instead.
        cycleContacts(true, false);
    }

    groupLayout.removeSortedWidget(chatroomWidget);
    chatroomWidget->deleteLater();
    groupList.remove(groupId);

    if (chatroomWidgetCount() == 0)
    {
        contentLayout->clear();
        activeChatroomWidget = nullptr;
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
        bool offlineEmpty = friendLayout->getLayoutOffline()->count() == 0;
        bool onlineEmpty = offlineEmpty && ((friendLayout->getLayoutOnline()->count() == 0 && groupsOnTop) || !groupsOnTop);
        bool groupsEmpty = offlineEmpty && ((groupLayout.getLayout()->count() == 0 && !groupsOnTop) || groupsOnTop);

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

        GenericChatroomWidget* chatWidget = dynamic_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());

        if (chatWidget != nullptr && chatWidget != activeChatroomWidget)
            onChatroomWidgetClicked(chatWidget, false);

        return;
    }
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

void ContentDialog::updateTitleUsername(const QString& username)
{
    if (displayWidget != nullptr)
        setWindowTitle(displayWidget->getTitle() + QStringLiteral(" - ") + username);
    else
        setWindowTitle(username);
}

void ContentDialog::updateTitle(GenericChatroomWidget* chatroomWidget)
{
    displayWidget = chatroomWidget;
    updateTitleUsername(Core::getInstance()->getUsername());
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

                if (frnd)
                {
                    frnd->getFriendWidget()->resetEventFlags();
                    frnd->getFriendWidget()->updateStatusLight();
                }
                else
                {
                    Group* g = activeChatroomWidget->getGroup();
                    g->getGroupWidget()->resetEventFlags();
                    g->getGroupWidget()->updateStatusLight();
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

void ContentDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("friend"))
    {
        int friendId = event->mimeData()->data("friend").toInt();
        auto iter = friendList.find(friendId);

        // If friend is already in a dialog then you can't drop friend where it already is.
        if (iter == friendList.end() || (iter != friendList.end() && std::get<0>(iter.value()) != this))
            event->acceptProposedAction();
    }
    else if (event->mimeData()->hasFormat("group"))
    {
        int groupId = event->mimeData()->data("group").toInt();
        auto iter = groupList.find(groupId);

        if (iter == groupList.end() || (iter != groupList.end() && std::get<0>(iter.value()) != this))
            event->acceptProposedAction();
    }
}

void ContentDialog::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("friend"))
    {
        int friendId = event->mimeData()->data("friend").toInt();
        auto iter = friendList.find(friendId);

        if (iter != friendList.end())
            std::get<0>(iter.value())->removeFriend(friendId);

        Friend* contact = FriendList::findFriend(friendId);
        Widget::getInstance()->addFriendDialog(contact, this);
        ensureSplitterVisible();
    }
    else if (event->mimeData()->hasFormat("group"))
    {
        int groupId = event->mimeData()->data("group").toInt();
        auto iter = friendList.find(groupId);

        if (iter != friendList.end())
            std::get<0>(iter.value())->removeGroup(groupId);

        Group* contact = GroupList::findGroup(groupId);
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
    QDialog::resizeEvent(event);
}

void ContentDialog::moveEvent(QMoveEvent* event)
{
    saveDialogGeometry();
    QDialog::moveEvent(event);
}

void ContentDialog::keyPressEvent(QKeyEvent* event)
{
    if(event->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(event); // Ignore escape keyboard shortcut.
}

void ContentDialog::onChatroomWidgetClicked(GenericChatroomWidget *widget, bool group)
{
    if (group)
    {
        ContentDialog* contentDialog = new ContentDialog(settingsWidget);
        contentDialog->show();

        if (widget->getFriend() != nullptr)
        {
            removeFriend(widget->getFriend()->getFriendID());
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

    contentLayout->clear();

    if (activeChatroomWidget != nullptr)
        activeChatroomWidget->setAsInactiveChatroom();

    activeChatroomWidget = widget;

    widget->setChatForm(contentLayout);
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
    updateTitleUsername(Core::getInstance()->getUsername());
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
