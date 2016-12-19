/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef FRIENDLISTLAYOUT_H
#define FRIENDLISTLAYOUT_H

#include <QBoxLayout>
#include "src/core/corestructs.h"
#include "genericchatitemlayout.h"

class FriendWidget;
class FriendListWidget;

class FriendListLayout : public QVBoxLayout
{
    Q_OBJECT
public:
    explicit FriendListLayout();
    explicit FriendListLayout(QWidget* parent);

    void addFriendWidget(FriendWidget* widget, Status s);
    void removeFriendWidget(FriendWidget* widget, Status s);
    int indexOfFriendWidget(GenericChatItemWidget* widget, bool online) const;
    void moveFriendWidgets(FriendListWidget* listWidget);
    int friendOnlineCount() const;
    int friendTotalCount() const;

    bool hasChatrooms() const;
    void searchChatrooms(const QString& searchString, bool hideOnline = false, bool hideOffline = false);

    QLayout* getLayoutOnline() const;
    QLayout* getLayoutOffline() const;

private:
    void init();
    QLayout* getFriendLayout(Status s) const;

    GenericChatItemLayout friendOnlineLayout;
    GenericChatItemLayout friendOfflineLayout;
};

#endif // FRIENDLISTLAYOUT_H
