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

#include "categorywidget.h"
#include "friendlistlayout.h"
#include "friendlistwidget.h"
#include "friendwidget.h"
#include "src/persistence/settings.h"
#include "src/widget/style.h"
#include "tool/croppinglabel.h"
#include <QBoxLayout>
#include <QMouseEvent>

#include <QApplication>

void CategoryWidget::emitChatroomWidget(QLayout* layout, int index)
{
    GenericChatroomWidget* chatWidget = qobject_cast<GenericChatroomWidget*>(layout->itemAt(index)->widget());
    if (chatWidget != nullptr)
        emit chatWidget->chatroomWidgetClicked(chatWidget);
}

CategoryWidget::CategoryWidget(QWidget* parent)
    : GenericChatItemWidget(parent)
{
    container = new QWidget(this);
    container->setObjectName("circleWidgetContainer");
    container->setLayoutDirection(Qt::LeftToRight);

    statusLabel = new QLabel(this);
    statusLabel->setObjectName("status");
    statusLabel->setTextFormat(Qt::PlainText);

    statusPic.setPixmap(QPixmap(":/ui/chatArea/scrollBarRightArrow.svg"));

    fullLayout = new QVBoxLayout(this);
    fullLayout->setSpacing(0);
    fullLayout->setMargin(0);
    fullLayout->addWidget(container);

    lineFrame = new QFrame(container);
    lineFrame->setObjectName("line");
    lineFrame->setFrameShape(QFrame::HLine);
    lineFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    lineFrame->resize(0, 0);

    listLayout = new FriendListLayout();
    listWidget = new QWidget(this);
    listWidget->setLayout(listLayout);
    fullLayout->addWidget(listWidget);

    setAcceptDrops(true);
    initLayout();
    setExpanded(true, false);
    updateStatus();
}

bool CategoryWidget::isExpanded() const
{
    return expanded;
}

void CategoryWidget::setExpanded(bool isExpanded, bool save)
{
    expanded = isExpanded;
    setMouseTracking(true);
    listWidget->setVisible(isExpanded);

    QString pixmapPath;
    if (isExpanded)
        pixmapPath = ":/ui/chatArea/scrollBarDownArrow.svg";
    else
        pixmapPath = ":/ui/chatArea/scrollBarRightArrow.svg";
    statusPic.setPixmap(QPixmap(pixmapPath));
    // The listWidget will recieve a enterEvent for some reason if now visible.
    // Using the following, we prevent that.
    QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers);
    container->hide();
    container->show();

    if (save)
        onExpand();
}

void CategoryWidget::leaveEvent(QEvent *event)
{
    event->ignore();
}

void CategoryWidget::setName(const QString &name, bool save)
{
    nameLabel->setText(name);

    const Settings& s = Settings::getInstance();
    if (s.getCompactLayout())
        nameLabel->minimizeMaximumWidth();

    if (save)
        onSetName();
}

void CategoryWidget::editName()
{
    nameLabel->editBegin();
    nameLabel->setMaximumWidth(QWIDGETSIZE_MAX);
}

void CategoryWidget::addFriendWidget(FriendWidget* w, Status s)
{
    listLayout->addFriendWidget(w, s);
    updateStatus();
    onAddFriendWidget(w);
    w->reloadTheme(); // Otherwise theme will change when moving to another circle.
}

void CategoryWidget::removeFriendWidget(FriendWidget* w, Status s)
{
    listLayout->removeFriendWidget(w, s);
    updateStatus();
}

void CategoryWidget::updateStatus()
{
    statusLabel->setText(QString::number(listLayout->friendOnlineCount()) + QStringLiteral(" / ") + QString::number(listLayout->friendTotalCount()));
}

bool CategoryWidget::hasChatrooms() const
{
    return listLayout->hasChatrooms();
}

void CategoryWidget::search(const QString &searchString, bool updateAll, bool hideOnline, bool hideOffline)
{
    if (updateAll)
    {
        listLayout->searchChatrooms(searchString, hideOnline, hideOffline);
    }
    bool inCategory = searchString.isEmpty() && !(hideOnline && hideOffline);
    setVisible(inCategory || listLayout->hasChatrooms());
}

bool CategoryWidget::cycleContacts(bool forward)
{
    if (listLayout->friendTotalCount() == 0)
    {
        return false;
    }
    if (forward)
    {
        if (!listLayout->getLayoutOnline()->isEmpty())
        {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOnline(), 0);
            return true;
        }
        else if (!listLayout->getLayoutOffline()->isEmpty())
        {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOffline(), 0);
            return true;
        }
    }
    else
    {
        if (!listLayout->getLayoutOffline()->isEmpty())
        {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOffline(), listLayout->getLayoutOffline()->count() - 1);
            return true;
        }
        else if (!listLayout->getLayoutOnline()->isEmpty())
        {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOnline(), listLayout->getLayoutOnline()->count() - 1);
            return true;
        }
    }
    return false;
}

bool CategoryWidget::cycleContacts(FriendWidget* activeChatroomWidget, bool forward)
{
    int index = -1;
    QLayout* currentLayout = nullptr;

    FriendWidget* friendWidget = qobject_cast<FriendWidget*>(activeChatroomWidget);
    if (friendWidget == nullptr)
        return false;

    currentLayout = listLayout->getLayoutOnline();
    index = listLayout->indexOfFriendWidget(friendWidget, true);
    if (index == -1)
    {
        currentLayout = listLayout->getLayoutOffline();
        index = listLayout->indexOfFriendWidget(friendWidget, false);
    }

    index += forward ? 1 : -1;
    for (;;)
    {
        // Bounds checking.
        if (index < 0)
        {
            if (currentLayout == listLayout->getLayoutOffline())
                currentLayout = listLayout->getLayoutOnline();
            else
                return false;

            index = currentLayout->count() - 1;
            continue;
        }
        else if (index >= currentLayout->count())
        {
            if (currentLayout == listLayout->getLayoutOnline())
                currentLayout = listLayout->getLayoutOffline();
            else
                return false;

            index = 0;
            continue;
        }

        GenericChatroomWidget* chatWidget = qobject_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());
        if (chatWidget != nullptr)
            emit chatWidget->chatroomWidgetClicked(chatWidget);

        break;
    }

    // TODO: method always returns true
    return true;
}

void CategoryWidget::initLayout()
{
    delete topLayout;
    delete mainLayout;

    topLayout = new QHBoxLayout;
    topLayout->setSpacing(0);
    topLayout->setMargin(0);

    nameLabel->minimizeMaximumWidth();

    mainLayout = nullptr;

    container->setFixedHeight(25);
    container->setLayout(topLayout);

    topLayout->addSpacing(18);
    topLayout->addWidget(&statusPic);
    topLayout->addSpacing(5);
    topLayout->addWidget(nameLabel, 100);
    topLayout->addWidget(lineFrame, 1);
    topLayout->addSpacing(5);
    topLayout->addWidget(statusLabel);
    topLayout->addSpacing(5);
    topLayout->activate();

    Style::repolish(this);
}

void CategoryWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        setExpanded(!expanded);
}

void CategoryWidget::setContainerAttribute(Qt::WidgetAttribute attribute, bool enabled)
{
    container->setAttribute(attribute, enabled);
    Style::repolish(container);
}

QLayout* CategoryWidget::friendOfflineLayout() const
{
    return listLayout->getLayoutOffline();
}

QLayout* CategoryWidget::friendOnlineLayout() const
{
    return listLayout->getLayoutOnline();
}

void CategoryWidget::moveFriendWidgets(FriendListWidget* friendList)
{
    listLayout->moveFriendWidgets(friendList);
}
