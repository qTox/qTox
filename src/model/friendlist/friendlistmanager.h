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

#include <memory>

class FriendListManager : public QObject
{    
    Q_OBJECT
public:
    using IFriendItemPtr = std::shared_ptr<IFriendListItem>;

    explicit FriendListManager(QObject *parent = nullptr);

    QVector<IFriendItemPtr> getItems() const;
    bool needHideCircles() const;

    void addFriendItem(IFriendListItem* item);
    void removeFriendItem(IFriendListItem* item);
    void sortByName();
    void sortByActivity();
    void resetParents();
    void setFilter(const QString& searchString, bool hideOnline,
                   bool hideOffline, bool hideGroups);
    void applyFilter();
    void updatePositions();    

    static void setGroupsOnTop(bool v);

signals:
    void itemsChanged();

private:
    struct FilterParams {
        QString searchString = "";
        bool hideOnline = false;
        bool hideOffline = false;
        bool hideGroups = false;
    } filterParams;

    void removeAll(IFriendListItem*);    
    static bool cmpByName(const IFriendItemPtr&, const IFriendItemPtr&);
    static bool cmpByActivity(const IFriendItemPtr&, const IFriendItemPtr&);    

    bool byName = true;
    bool hideCircles = false;
    static bool groupsOnTop;
    QVector<IFriendItemPtr> items;

};
