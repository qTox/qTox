/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#pragma once

#include "genericchatitemwidget.h"
#include "src/core/core.h"
#include "src/model/status.h"

class FriendListLayout;
class FriendListWidget;
class FriendWidget;
class QVBoxLayout;
class QHBoxLayout;

class CategoryWidget : public GenericChatItemWidget
{
    Q_OBJECT
public:
    explicit CategoryWidget(bool compact, QWidget* parent = nullptr);

    bool isExpanded() const;
    void setExpanded(bool isExpanded, bool save = true);
    void setName(const QString& name, bool save = true);

    void addFriendWidget(FriendWidget* w, Status::Status s);
    void removeFriendWidget(FriendWidget* w, Status::Status s);
    void updateStatus();

    bool hasChatrooms() const;
    bool cycleContacts(bool forward);
    bool cycleContacts(FriendWidget* activeChatroomWidget, bool forward);
    void search(const QString& searchString, bool updateAll = false, bool hideOnline = false,
                bool hideOffline = false);

public slots:
    void onCompactChanged(bool compact);
    void moveFriendWidgets(FriendListWidget* friendList);

protected:
    void leaveEvent(QEvent* event) final;
    void mouseReleaseEvent(QMouseEvent* event) final;

    void editName();
    void setContainerAttribute(Qt::WidgetAttribute attribute, bool enabled);
    QLayout* friendOnlineLayout() const;
    QLayout* friendOfflineLayout() const;
    void emitChatroomWidget(QLayout* layout, int index);

private:
    virtual void onSetName()
    {
    }
    virtual void onExpand()
    {
    }
    virtual void onAddFriendWidget(FriendWidget*)
    {
    }

    QWidget* listWidget;
    FriendListLayout* listLayout;
    QVBoxLayout* fullLayout;
    QVBoxLayout* mainLayout = nullptr;
    QHBoxLayout* topLayout = nullptr;
    QLabel* statusLabel;
    QWidget* container;
    QFrame* lineFrame;
    bool expanded = false;
};
