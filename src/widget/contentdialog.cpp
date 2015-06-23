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
#include "src/widget/form/chatform.h"
#include "src/core/core.h"
#include <QBoxLayout>
#include <QSplitter>
#include <QGuiApplication>

#include <QDragEnterEvent>
#include <QMimeData>

#include <QDebug>

/*class ContentFriendList : public QScrollArea
{
public:
    ContentFriendList(QWidget* parent = 0) : QScrollArea(parent)
    {
        setAcceptDrops(true);
    }

protected:

    void dragEnterEvent(QDragEnterEvent* event) final override
    {
        qDebug() << "h";
        if (event->mimeData()->hasFormat("friend"))
            event->acceptProposedAction();
        QScrollArea::dragEnterEvent(event);
    }

    void dropEvent(QDropEvent *event) final override
    {
        if (event->mimeData()->hasFormat("friend"))
        {
            int friendId = ev->mimeData()->data("friend").toInt();
            //Core::getInstance()->groupInviteFriend(friendId, groupId);
        }
    }
};*/

ContentDialog* ContentDialog::currentDialog = nullptr;
QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> ContentDialog::friendList;
QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> ContentDialog::groupList;

ContentDialog::ContentDialog(QWidget* parent)
    : QDialog(parent, Qt::Window)
    , activeChatroomWidget(nullptr)
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

    friendLayout = new QVBoxLayout(friendWidget);
    friendLayout->setMargin(0);
    friendLayout->setSpacing(0);
    friendLayout->addStretch();

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

    setMinimumSize(775, 420);
    setAttribute(Qt::WA_DeleteOnClose);

    //restore window state
    restoreGeometry(Settings::getInstance().getDialogGeometry());
    splitter->restoreState(Settings::getInstance().getDialogSplitterState());

    currentDialog = this;

    setAcceptDrops(true);
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
}

FriendWidget* ContentDialog::addFriend(int friendId, QString id)
{
    FriendWidget* friendWidget = new FriendWidget(friendId, id);
    friendLayout->insertWidget(friendLayout->count() - 1, friendWidget);

    Friend* frnd = friendWidget->getFriend();

    onChatroomWidgetClicked(friendWidget, false);

    connect(frnd, &Friend::displayedNameChanged, this, &ContentDialog::updateFriendWidget);
    connect(friendWidget, &FriendWidget::chatroomWidgetClicked, this, &ContentDialog::onChatroomWidgetClicked);
    connect(friendWidget, SIGNAL(chatroomWidgetClicked(GenericChatroomWidget*)), frnd->getChatForm(), SLOT(focusInput()));
    connect(Core::getInstance(), &Core::friendAvatarChanged, friendWidget, &FriendWidget::onAvatarChange);
    connect(Core::getInstance(), &Core::friendAvatarRemoved, friendWidget, &FriendWidget::onAvatarRemoved);

    friendList.insert(friendId, std::make_tuple(this, friendWidget));

    return friendWidget;
}

GroupWidget* ContentDialog::addGroup(int groupId, const QString& name)
{
    GroupWidget* groupWidget = new GroupWidget(groupId, name);
    friendLayout->insertWidget(friendLayout->count() - 1, groupWidget);

    Group* group = groupWidget->getGroup();
    connect(group, &Group::titleChanged, this, &ContentDialog::updateGroupWidget);
    connect(group, &Group::userListChanged, this, &ContentDialog::updateGroupWidget);
    connect(groupWidget, &GroupWidget::chatroomWidgetClicked, this, &ContentDialog::onChatroomWidgetClicked);

    onChatroomWidgetClicked(groupWidget, false);

    groupList.insert(groupId, std::make_tuple(this, groupWidget));

    return groupWidget;
}

void ContentDialog::removeFriend(int friendId)
{
    remove(friendId, friendList);
}

void ContentDialog::removeGroup(int groupId)
{
    remove(groupId, groupList);
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
    return friendLayout->count() - 1; // Don't include the stretch.
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

        // Display friend list after dropping, if not already visible.
        if (splitter->sizes().at(0) == 0)
            splitter->setSizes({1, 1});
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
    Q_UNUSED(event);
    saveDialogGeometry();
}

void ContentDialog::closeEvent(QCloseEvent* event)
{
    saveDialogGeometry();
    saveSplitterState();
    QWidget::closeEvent(event);
}

void ContentDialog::onChatroomWidgetClicked(GenericChatroomWidget *widget, bool group)
{
    if (group)
    {
        ContentDialog* contentDialog = new ContentDialog();
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
    setWindowTitle(widget->getTitle());

    if (widget->getFriend() != nullptr)
        widget->getFriend()->getFriendWidget()->updateStatusLight();
    else
        widget->getGroup()->getGroupWidget()->updateStatusLight();
}

void ContentDialog::updateFriendWidget(FriendWidget *w, Status s)
{
    // Note: This argument friend widget is not the one stored here!
    Q_UNUSED(s); // Use it after friend list improvements are merged, to sort contacts here.

    std::get<1>(friendList.find(w->friendId).value())->setName(w->getName());
}

void ContentDialog::updateGroupWidget(GroupWidget *w)
{
    std::get<1>(groupList.find(w->groupId).value())->setName(w->getName());
    static_cast<GroupWidget*>(std::get<1>(groupList.find(w->groupId).value()))->onUserListChanged();
}

void ContentDialog::saveDialogGeometry()
{
    Settings::getInstance().setDialogGeometry(saveGeometry());
}

void ContentDialog::saveSplitterState()
{
    Settings::getInstance().setDialogSplitterState(splitter->saveState());
}

void ContentDialog::remove(int id, const QHash<int, std::tuple<ContentDialog *, GenericChatroomWidget *> > &list)
{
    auto iter = list.find(id);

    if (iter == list.end())
        return;

    GenericChatroomWidget* chatroomWidget = std::get<1>(iter.value());

    if (activeChatroomWidget == chatroomWidget)
    {
        // Need to find replacement to show here instead.
        if (chatroomWidgetCount() > 1)
        {
            int index = friendLayout->indexOf(chatroomWidget) - 1;

            // Don't let it go below 0. If it does, then we're first. Go second.
            if (index < 0)
                index = 1;

            GenericChatroomWidget* chatroomWidget = static_cast<GenericChatroomWidget*>(friendLayout->itemAt(index)->widget());
            onChatroomWidgetClicked(chatroomWidget, false);
        }
        else
        {
            contentLayout->clear();
            activeChatroomWidget = nullptr;
            deleteLater();
        }
    }

    friendLayout->removeWidget(chatroomWidget);
    chatroomWidget->deleteLater();
    friendList.remove(id);
    update();
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
        std::get<0>(iter.value())->setWindowTitle(chatroomWidget->getTitle());
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
