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

#include "util/interface.h"

#include <QStringList>

class IGroupSettings
{
public:
    IGroupSettings() = default;
    virtual ~IGroupSettings();
    IGroupSettings(const IGroupSettings&) = default;
    IGroupSettings& operator=(const IGroupSettings&) = default;
    IGroupSettings(IGroupSettings&&) = default;
    IGroupSettings& operator=(IGroupSettings&&) = default;

    virtual QStringList getBlackList() const = 0;
    virtual void setBlackList(const QStringList& blist) = 0;

    virtual bool getShowGroupJoinLeaveMessages() const = 0;
    virtual void setShowGroupJoinLeaveMessages(bool newValue) = 0;

    DECLARE_SIGNAL(blackListChanged, QStringList const& blist);
    DECLARE_SIGNAL(showGroupJoinLeaveMessagesChanged, bool show);
};
