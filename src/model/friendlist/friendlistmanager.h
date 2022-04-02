/*
    Copyright © 2021 by The qTox Project Contributors

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

#include "ifriendlistitem.h"

#include <QObject>
#include <QVector>

#include <memory>

class FriendListManager : public QObject
{
    Q_OBJECT
public:
    using IFriendListItemPtr = std::shared_ptr<IFriendListItem>;

    explicit FriendListManager(int countContacts_, QObject *parent = nullptr);

    QVector<IFriendListItemPtr> getItems() const;
    bool needHideCircles() const;
    // If the contact positions have changed, need to redraw view
    bool getPositionsChanged() const;
    bool getGroupsOnTop() const;

    void addFriendListItem(IFriendListItem* item);
    void removeFriendListItem(IFriendListItem* item);
    void sortByName();
    void sortByActivity();
    void resetParents();
    void setFilter(const QString& searchString, bool hideOnline,
                   bool hideOffline, bool hideGroups);
    void applyFilter();
    void updatePositions();
    void setSortRequired();

    void setGroupsOnTop(bool v);

signals:
    void itemsChanged();

private:
    struct FilterParams {
        QString searchString = "";
        bool hideOnline = false;
        bool hideOffline = false;
        bool hideGroups = false;
    } filterParams;

    void removeAll(IFriendListItem* item);
    bool cmpByName(const IFriendListItemPtr& itemA, const IFriendListItemPtr& itemB);
    bool cmpByActivity(const IFriendListItemPtr& itemA, const IFriendListItemPtr& itemB);

    bool byName = true;
    bool hideCircles = false;
    bool groupsOnTop = true;
    bool positionsChanged = false;
    bool needSort = false;
    QVector<IFriendListItemPtr> items;
    // At startup, while the size of items is less than countContacts, the view will not be processed to improve performance
    int countContacts = 0;

};
