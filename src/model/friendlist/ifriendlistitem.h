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

#include <QDate>

class QWidget;

class IFriendListItem
{
public:
    IFriendListItem() = default;
    virtual ~IFriendListItem();
    IFriendListItem(const IFriendListItem&) = default;
    IFriendListItem& operator=(const IFriendListItem&) = default;
    IFriendListItem(IFriendListItem&&) = default;
    IFriendListItem& operator=(IFriendListItem&&) = default;

    virtual bool isFriend() const = 0;
    virtual bool isGroup() const = 0;
    virtual bool isOnline() const = 0;
    virtual bool widgetIsVisible() const = 0;
    virtual QString getNameItem() const = 0;
    virtual QDateTime getLastActivity() const = 0;
    virtual QWidget* getWidget() = 0;
    virtual void setWidgetVisible(bool) = 0;

    virtual int getCircleId() const
    {
        return -1;
    }

    int getNameSortedPos() const
    {
        return nameSortedPos;
    }

    void setNameSortedPos(int pos)
    {
        nameSortedPos = pos;
    }

private:
    int nameSortedPos = -1;
};
