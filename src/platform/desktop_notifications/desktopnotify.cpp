/*
    Copyright © 2019 by The qTox Project Contributors

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

#if DESKTOP_NOTIFICATIONS
#include "desktopnotify.h"

#include <src/persistence/settings.h>

#include <KNotifications/KNotification>
#include <KF5/knotifications_version.h>

#include <QDebug>
#include <QThread>
#include <QStandardPaths>
#include <QWidget>

DesktopNotify::DesktopNotify(QWidget* parent)
    : parent(parent)
{}

void DesktopNotify::notifyMessage(const NotificationData& notificationData)
{

    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    if (!notification) {
        notification = new KNotification("generic", KNotification::CloseWhenWidgetActivated);
        notification->setWidget(parent);

#if KNOTIFICATIONS_VERSION < (5<<16|67<<8)
        // Default action only supported on 5.67 or later
        notification->setActions({tr("Open")});
        connect(notification, &KNotification::action1Activated, this, &DesktopNotify::onNotificationActivated);
#else
        notification->setDefaultAction({tr("Open")});
        connect(notification, &KNotification::defaultActivated, this, &DesktopNotify::onNotificationActivated);
#endif
        connect(notification, &KNotification::closed, this, &DesktopNotify::onNotificationClosed);
        connect(notification, &KNotification::ignored, this, &DesktopNotify::onNotificationClosed);
    }

    notification->setTitle(notificationData.title);
    notification->setText(notificationData.message);

    // NOTE: On some systems this does not seem to be displayed. On GNOME 3.38
    // the notification displays the icon whether or not KNotifications provides
    // the pixmap under the "image_data" item in the dbus request. According to
    // https://developer.gnome.org/notification-spec/ image-data should be
    // highest priority, however it is not being chosen even though it is
    // visible in dbus-monitor
    //
    // NOTE2: If notificationData.pixmap isNull KNotification will send the qtox
    // icon instead
    notification->setPixmap(notificationData.pixmap);

    notification->sendEvent();
}

void DesktopNotify::onNotificationActivated()
{
    notification = nullptr;
    emit notificationActivated();
}

void DesktopNotify::onNotificationClosed()
{
    notification = nullptr;
    emit notificationClosed();
}

#endif
