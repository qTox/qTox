/*
    Copyright © 2014-2015 by The qTox Project

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

#ifndef FRIENDLISTWIDGET_H
#define FRIENDLISTWIDGET_H

#include <QMap>
#include <QWidget>

#include "src/core/corestructs.h"
#include "src/friend.h"
#include "src/group.h"
#include "genericchatitemlayout.h"

class QVBoxLayout;
class QGridLayout;
class QPixmap;
class FriendListLayout;
class FriendWidget;
class GroupWidget;
class CircleWidget;
class GenericChatroomWidget;
class Widget;

class FriendListWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode : uint8_t
    {
        Name,
        Activity,
    };

    explicit FriendListWidget(Widget* parent, bool groupsOnTop = true);
    ~FriendListWidget();

    void setMode(Mode mode);
    Mode getMode() const;

    void clear();

    GroupWidget* getGroupWidget(int groupId) const;
    GroupWidget* requestGroupWidget(int groupId);
    FriendWidget* getFriendWidget(uint32_t friendId) const;
    FriendWidget* requestFriendWidget(const Friend& f, int circleIndex);
    void addCircleWidget(int id);
    void addCircleWidget(FriendWidget* widget = nullptr);
    void removeCircleWidget(CircleWidget* widget);
    void searchChatrooms(const QString &searchString, bool hideOnline = false, bool hideOffline = false, bool hideGroups = false);

    void cycleContacts(GenericChatroomWidget* activeChatroomWidget, bool forward);

    void updateActivityDate(const QDate& date);
    void reloadTheme();

public slots:
    void renameCircleWidget(CircleWidget* circleWidget, const QString& newName);
    void onGroupchatPositionChanged(bool top);
    void moveWidget(FriendWidget* w, Status s, bool add = false);

private slots:
    void onFriendRemoved(uint32_t friendId);
    void onFriendAliasChanged(const Friend& f, QString alias);
    void onFriendAvatarChanged(const Friend& f, const QPixmap& avatar);
    void onFriendStatusChanged(const Friend& f, Status status);
    void onFriendStatusMessageChanged(const Friend& f, QString message);
    void onGroupRemoved(int groupId);
    void dayTimeout();

private:
    void removeFriendWidget(uint32_t friendId);
    void removeGroupWidget(int groupId);

private:
    void dragEnterEvent(QDragEnterEvent* event) override final;
    void dropEvent(QDropEvent* event) override final;

private:
    QMap<Friend::ID, FriendWidget*> friendWidgets;
    QMap<Group::ID, GroupWidget*> groupWidgets;

    CircleWidget* createCircleWidget(int id = -1);
    QLayout* nextLayout(QLayout* layout, bool forward) const;
    void moveFriends(QLayout *layout);

    Mode mode;
    bool groupsOnTop;
    FriendListLayout* listLayout;
    GenericChatItemLayout* circleLayout = nullptr;
    GenericChatItemLayout groupLayout;
    QVBoxLayout* activityLayout = nullptr;
    QTimer* dayTimer;
};

#endif // FRIENDLISTWIDGET_H
