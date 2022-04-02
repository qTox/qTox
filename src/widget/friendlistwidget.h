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
class FriendListManager;
class GenericChatroomWidget;
class CategoryWidget;
class Friend;
class IFriendListItem;
class Settings;
class Style;
class IMessageBoxManager;
class FriendList;
class GroupList;
class Profile;

class FriendListWidget : public QWidget
{
    Q_OBJECT
public:
    using SortingMode = Settings::FriendListSortingMode;
    FriendListWidget(const Core& core, Widget* parent, Settings& settings, Style& style,
        IMessageBoxManager& messageBoxManager, FriendList& friendList, GroupList& groupList,
        Profile& profile, bool groupsOnTop = true);
    ~FriendListWidget();
    void setMode(SortingMode mode);
    SortingMode getMode() const;

    void addGroupWidget(GroupWidget* widget);
    void addFriendWidget(FriendWidget* w);
    void removeGroupWidget(GroupWidget* w);
    void removeFriendWidget(FriendWidget* w);
    void addCircleWidget(int id);
    void addCircleWidget(FriendWidget* widget = nullptr);
    void removeCircleWidget(CircleWidget* widget);
    void searchChatrooms(const QString& searchString, bool hideOnline = false,
                         bool hideOffline = false, bool hideGroups = false);

    void cycleChats(GenericChatroomWidget* activeChatroomWidget, bool forward);

    void updateActivityTime(const QDateTime& time);

signals:
    void onCompactChanged(bool compact);
    void connectCircleWidget(CircleWidget& circleWidget);
    void searchCircle(CircleWidget& circleWidget);

public slots:
    void renameGroupWidget(GroupWidget* groupWidget, const QString& newName);
    void renameCircleWidget(CircleWidget* circleWidget, const QString& newName);
    void onGroupchatPositionChanged(bool top);
    void moveWidget(FriendWidget* w, Status::Status s, bool add = false);
    void itemsChanged();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void dayTimeout();

private:
    CircleWidget* createCircleWidget(int id = -1);
    CategoryWidget* getTimeCategoryWidget(const Friend* frd) const;
    void sortByMode();
    void cleanMainLayout();
    QWidget* getNextWidgetForName(IFriendListItem* currentPos, bool forward) const;
    QVector<std::shared_ptr<IFriendListItem> > getItemsFromCircle(CircleWidget* circle) const;

    SortingMode mode;
    QVBoxLayout* listLayout = nullptr;
    QVBoxLayout* activityLayout = nullptr;
    QTimer* dayTimer;
    FriendListManager* manager;

    const Core& core;
    Settings& settings;
    Style& style;
    IMessageBoxManager& messageBoxManager;
    FriendList& friendList;
    GroupList& groupList;
    Profile& profile;
};
