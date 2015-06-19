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
#include "style.h"
#include "tool/adjustingscrollarea.h"
#include "src/persistence/settings.h"
#include "src/friend.h"
#include "src/friendlist.h"
#include <QBoxLayout>
#include <QSplitter>

ContentDialog* ContentDialog::currentDialog = nullptr;
QHash<int, std::tuple<ContentDialog*, GenericChatroomWidget*>> ContentDialog::friendList;

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

void ContentDialog::addFriend(int friendId, QString id)
{
    FriendWidget* friendWidget = new FriendWidget(friendId, id);
    friendLayout->insertWidget(friendLayout->count() - 1, friendWidget);

    onChatroomWidgetClicked(friendWidget);

    connect(friendWidget, &FriendWidget::chatroomWidgetClicked, this, &ContentDialog::onChatroomWidgetClicked);

    friendList.insert(friendId, std::make_tuple(this, friendWidget));
}

ContentDialog* ContentDialog::current()
{
    return currentDialog;
}

bool ContentDialog::showChatroomWidget(int friendId)
{
    auto widgetIt = friendList.find(friendId);
    if (widgetIt == friendList.end())
        return false;

    std::get<0>(widgetIt.value())->activateWindow();
    std::get<0>(widgetIt.value())->onChatroomWidgetClicked(std::get<1>(widgetIt.value()));

    return true;
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

void ContentDialog::onChatroomWidgetClicked(GenericChatroomWidget *widget)
{
    contentLayout->clear();

    if (activeChatroomWidget != nullptr)
        activeChatroomWidget->setAsInactiveChatroom();

    activeChatroomWidget = widget;

    widget->setChatForm(contentLayout);
    setWindowTitle(widget->getName());
    widget->setAsActiveChatroom();
    widget->resetEventFlags();
    widget->updateStatusLight();
    QString windowTitle = widget->getName();
    if (!widget->getStatusString().isNull())
        windowTitle += " (" + widget->getStatusString() + ")";
    setWindowTitle(windowTitle);
}

void ContentDialog::saveDialogGeometry()
{
    Settings::getInstance().setDialogGeometry(saveGeometry());
}

void ContentDialog::saveSplitterState()
{
    Settings::getInstance().setDialogSplitterState(splitter->saveState());
}
