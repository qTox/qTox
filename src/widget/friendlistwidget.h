/*
    Copyright © 2014-2019 by The qTox Project Contributors

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
#include "src/core/core.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include <QWidget>

class QVBoxLayout;
class QGridLayout;
class QPixmap;
class Widget;
class FriendWidget;
class GroupWidget;
class CircleWidget;
class FriendListLayout;
class GenericChatroomWidget;
class CategoryWidget;
class Friend;

class FriendListWidget : public QWidget
{
    Q_OBJECT
public:
    using SortingMode = Settings::FriendListSortingMode;
    explicit FriendListWidget(Widget* parent, bool groupsOnTop = true);
    ~FriendListWidget();
    void setMode(SortingMode mode);
    SortingMode getMode() const;

    void addGroupWidget(GroupWidget* widget);
    void addFriendWidget(FriendWidget* w, Status::Status s, int circleIndex);
    void removeGroupWidget(GroupWidget* w);
    void removeFriendWidget(FriendWidget* w);
    void addCircleWidget(int id);
    void addCircleWidget(FriendWidget* widget = nullptr);
    void removeCircleWidget(CircleWidget* widget);
    void searchChatrooms(const QString& searchString, bool hideOnline = false,
                         bool hideOffline = false, bool hideGroups = false);

    void cycleContacts(GenericChatroomWidget* activeChatroomWidget, bool forward);

    void updateActivityTime(const QDateTime& date);
    void reDraw();

signals:
    void onCompactChanged(bool compact);
    void connectCircleWidget(CircleWidget& circleWidget);
    void searchCircle(CircleWidget& circleWidget);

public slots:
    void renameGroupWidget(GroupWidget* groupWidget, const QString& newName);
    void renameCircleWidget(CircleWidget* circleWidget, const QString& newName);
    void onFriendWidgetRenamed(FriendWidget* friendWidget);
    void onGroupchatPositionChanged(bool top);
    void moveWidget(FriendWidget* w, Status::Status s, bool add = false);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void dayTimeout();

private:
    CircleWidget* createCircleWidget(int id = -1);
    QLayout* nextLayout(QLayout* layout, bool forward) const;
    void moveFriends(QLayout* layout);
    CategoryWidget* getTimeCategoryWidget(const Friend* frd) const;
    void sortByMode(SortingMode mode);

    SortingMode mode;
    bool groupsOnTop;
    FriendListLayout* listLayout;
    GenericChatItemLayout* circleLayout = nullptr;
    GenericChatItemLayout groupLayout;
    QVBoxLayout* activityLayout = nullptr;
    QTimer* dayTimer;
};
