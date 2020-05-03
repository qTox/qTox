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

#pragma once

#include "genericchatitemlayout.h"
#include "src/model/status.h"
#include "src/core/core.h"
#include <QBoxLayout>

class FriendWidget;
class FriendListWidget;

class FriendListLayout : public QVBoxLayout
{
    Q_OBJECT
public:
    explicit FriendListLayout();
    explicit FriendListLayout(QWidget* parent);

    void addFriendWidget(FriendWidget* widget, Status::Status s);
    void removeFriendWidget(FriendWidget* widget, Status::Status s);
    int indexOfFriendWidget(GenericChatItemWidget* widget, bool online) const;
    void moveFriendWidgets(FriendListWidget* listWidget);
    int friendOnlineCount() const;
    int friendTotalCount() const;

    bool hasChatrooms() const;
    void searchChatrooms(const QString& searchString, bool hideOnline = false,
                         bool hideOffline = false);

    QLayout* getLayoutOnline() const;
    QLayout* getLayoutOffline() const;

private:
    void init();
    QLayout* getFriendLayout(Status::Status s) const;

    GenericChatItemLayout friendOnlineLayout;
    GenericChatItemLayout friendOfflineLayout;
};
