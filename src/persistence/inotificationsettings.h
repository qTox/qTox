/*
    Copyright © 2020 by The qTox Project Contributors

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

#include <QStringList>

class INotificationSettings
{
public:
    INotificationSettings() = default;
    virtual ~INotificationSettings();
    INotificationSettings(const INotificationSettings&) = default;
    INotificationSettings& operator=(const INotificationSettings&) = default;
    INotificationSettings(INotificationSettings&&) = default;
    INotificationSettings& operator=(INotificationSettings&&) = default;

    virtual bool getNotify() const = 0;
    virtual void setNotify(bool newValue) = 0;

    virtual bool getShowWindow() const = 0;
    virtual void setShowWindow(bool newValue) = 0;

    virtual bool getDesktopNotify() const = 0;
    virtual void setDesktopNotify(bool enabled) = 0;

    virtual bool getNotifySound() const = 0;
    virtual void setNotifySound(bool newValue) = 0;

    virtual bool getNotifyHide() const = 0;
    virtual void setNotifyHide(bool newValue) = 0;

    virtual bool getBusySound() const = 0;
    virtual void setBusySound(bool newValue) = 0;

    virtual bool getGroupAlwaysNotify() const = 0;
    virtual void setGroupAlwaysNotify(bool newValue) = 0;
};
