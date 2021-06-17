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

#include "friendlistmanager.h"
#include "src/widget/genericchatroomwidget.h"

bool FriendListManager::groupsOnTop = true;

FriendListManager::FriendListManager(QObject *parent) : QObject(parent)
{

}

QVector<FriendListManager::IFriendItemPtr> FriendListManager::getItems() const
{
    return items;
}

bool FriendListManager::needHideCircles() const
{
    return hideCircles;
}

void FriendListManager::addFriendItem(IFriendListItem *item)
{
    removeAll(item);

    if (item->isGroup()) {
        items.push_back(IFriendItemPtr(item, [](IFriendListItem* groupItem){
                            groupItem->getWidget()->deleteLater();}));
    } else {
        items.push_back(IFriendItemPtr(item));
    }

    updatePositions();
    emit itemsChanged();
}

void FriendListManager::removeFriendItem(IFriendListItem *item)
{
    removeAll(item);
    updatePositions();
    emit itemsChanged();
}

void FriendListManager::sortByName()
{
    byName = true;
    updatePositions();
}

void FriendListManager::sortByActivity()
{
    byName = false;
    updatePositions();
}

void FriendListManager::resetParents()
{
    for (int i = 0; i < items.size(); ++i) {
        IFriendListItem* itemTmp = items[i].get();
        itemTmp->getWidget()->setParent(nullptr);
    }
}

void FriendListManager::setFilter(const QString &searchString, bool hideOnline, bool hideOffline,
                                  bool hideGroups)
{
    if (filterParams.searchString == searchString && filterParams.hideOnline == hideOnline &&
        filterParams.hideOffline == hideOffline && filterParams.hideGroups == hideGroups) {
        return;
    }
    filterParams.searchString = searchString;
    filterParams.hideOnline = hideOnline;
    filterParams.hideOffline = hideOffline;
    filterParams.hideGroups = hideGroups;

    emit itemsChanged();
}

void FriendListManager::applyFilter()
{
    QString searchString = filterParams.searchString;

    for (IFriendItemPtr itemTmp : items) {
        if (searchString.isEmpty()) {
            itemTmp->getWidget()->setVisible(true);
        } else {
            QString tmp_name = itemTmp->getNameItem();
            itemTmp->getWidget()->setVisible(tmp_name.contains(searchString, Qt::CaseInsensitive));
        }

        if (filterParams.hideOnline && itemTmp->isOnline()) {
            if (itemTmp->isFriend()) {
                itemTmp->getWidget()->setVisible(false);
            }
        }

        if (filterParams.hideOffline && !itemTmp->isOnline()) {
            itemTmp->getWidget()->setVisible(false);
        }

        if (filterParams.hideGroups && itemTmp->isGroup()) {
            itemTmp->getWidget()->setVisible(false);
        }
    }

    if (filterParams.hideOnline && filterParams.hideOffline) {
        hideCircles = true;
    } else {
        hideCircles = false;
    }
}

void FriendListManager::updatePositions()
{
    if (byName) {
        std::sort(items.begin(), items.end(), cmpByName);
    } else {
        std::sort(items.begin(), items.end(), cmpByActivity);
    }
}

void FriendListManager::setGroupsOnTop(bool v)
{
    groupsOnTop = v;
}

void FriendListManager::removeAll(IFriendListItem* item)
{
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].get() == item) {
            items.remove(i);
            --i;
        }
    }
}

bool FriendListManager::cmpByName(const IFriendItemPtr &a, const IFriendItemPtr &b)
{
    if (a->isGroup() && !b->isGroup()) {
        if (groupsOnTop) {
            return true;
        }
        return !b->isOnline();
    }

    if (!a->isGroup() && b->isGroup()) {
        if (groupsOnTop) {
            return false;
        }
        return a->isOnline();
    }

    if (a->isOnline() && !b->isOnline()) {
        return true;
    }

    if (!a->isOnline() && b->isOnline()) {
        return false;
    }

    return a->getNameItem().toUpper() < b->getNameItem().toUpper();
}

bool FriendListManager::cmpByActivity(const IFriendItemPtr &a, const IFriendItemPtr &b)
{
    if (a->isGroup() || b->isGroup()) {
        if (a->isGroup() && !b->isGroup()) {
            return true;
        }

        if (!a->isGroup() && b->isGroup()) {
            return false;
        }
        return a->getNameItem().toUpper() < b->getNameItem().toUpper();
    }

    QDateTime dateA = a->getLastActivity();
    QDateTime dateB = b->getLastActivity();
    if (dateA.date() == dateB.date()) {
        if (a->isOnline() && !b->isOnline()) {
            return true;
        }

        if (!a->isOnline() && b->isOnline()) {
            return false;
        }
        return a->getNameItem().toUpper() < b->getNameItem().toUpper();
    }

    return a->getLastActivity() > b->getLastActivity();
}

