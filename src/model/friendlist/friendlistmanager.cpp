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

FriendListManager::FriendListManager(int countContacts_, QObject *parent) : QObject(parent)
{
    countContacts = countContacts_;
}

QVector<FriendListManager::IFriendListItemPtr> FriendListManager::getItems() const
{
    return items;
}

bool FriendListManager::needHideCircles() const
{
    return hideCircles;
}

bool FriendListManager::getPositionsChanged() const
{
    return positionsChanged;
}

bool FriendListManager::getGroupsOnTop() const
{
    return groupsOnTop;
}

void FriendListManager::addFriendListItem(IFriendListItem *item)
{
    if (item->isGroup() && item->getWidget() != nullptr) {
        items.push_back(IFriendListItemPtr(item, [](IFriendListItem* groupItem){
                            groupItem->getWidget()->deleteLater();}));
    } else {
        items.push_back(IFriendListItemPtr(item));
    }

    if (countContacts <= items.size()) {
        countContacts = 0;
        setSortRequired();
    }
}

void FriendListManager::removeFriendListItem(IFriendListItem *item)
{
    removeAll(item);
    setSortRequired();
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

    setSortRequired();
}

void FriendListManager::applyFilter()
{
    QString searchString = filterParams.searchString;

    for (IFriendListItemPtr itemTmp : items) {
        if (searchString.isEmpty()) {
            itemTmp->setWidgetVisible(true);
        } else {
            QString tmp_name = itemTmp->getNameItem();
            itemTmp->setWidgetVisible(tmp_name.contains(searchString, Qt::CaseInsensitive));
        }

        if (filterParams.hideOnline && itemTmp->isOnline()) {
            if (itemTmp->isFriend()) {
                itemTmp->setWidgetVisible(false);
            }
        }

        if (filterParams.hideOffline && !itemTmp->isOnline()) {
            itemTmp->setWidgetVisible(false);
        }

        if (filterParams.hideGroups && itemTmp->isGroup()) {
            itemTmp->setWidgetVisible(false);
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
    positionsChanged = true;

    if (byName) {
        auto sortName = [&](const IFriendListItemPtr &a, const IFriendListItemPtr &b) {
                            return cmpByName(a, b);
                        };
        if (!needSort) {
            if (std::is_sorted(items.begin(), items.end(), sortName)) {
                positionsChanged = false;
                return;
            }
        }
        std::sort(items.begin(), items.end(), sortName);

    } else {
        auto sortActivity = [&](const IFriendListItemPtr &a, const IFriendListItemPtr &b) {
                                return cmpByActivity(a, b);
                            };
        if (!needSort) {
            if (std::is_sorted(items.begin(), items.end(), sortActivity)) {
                positionsChanged = false;
                return;
            }
        }
        std::sort(items.begin(), items.end(), sortActivity);
    }

    needSort = false;
}

void FriendListManager::setSortRequired()
{
    needSort = true;
    emit itemsChanged();
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

bool FriendListManager::cmpByName(const IFriendListItemPtr &a, const IFriendListItemPtr &b)
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

bool FriendListManager::cmpByActivity(const IFriendListItemPtr &a, const IFriendListItemPtr &b)
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
