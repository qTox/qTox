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
#include <knotifications_version.h>

#include <QDebug>
#include <QThread>
#include <QStandardPaths>
#include <QWidget>
#include <QFile>
#include <QDir>

#include <fstream>

namespace
{
    void installNotificationConfig()
    {
        constexpr auto filename = "qTox.notifyrc";
        constexpr auto knotificationsDirName = "knotifications5";
        const auto genericDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        const auto configDirPath =  genericDataPath + "/" + knotificationsDirName;
        const auto configFilePath = configDirPath + "/" + filename;

        QFile configFile(configFilePath);

        // If we update this later we'll have to do something more intelligent
        // like hash both files and overwrite if our hash doesn't match the one
        // on disk
        if (configFile.exists())
            return;

        QDir genericDataDir(genericDataPath);

        genericDataDir.mkdir(knotificationsDirName);

        QFile inputFile(":qTox.notifyrc");
        inputFile.copy(configFilePath);
    }
} // namespace

DesktopNotify::DesktopNotify(QWidget* parent)
    : parent(parent)
{
    // Our mac OS installer is a dmg which as far as I can tell cannot install
    // our config to the required location. This is because GenericDataLocation
    // is not within the Application folder we extract to. Short of switching to
    // a real installer I am unsure of how to get our config file to the right
    // spot at install time.
    //
    // Since we have this code anyways on all platforms we just install the
    // config file at runtime on all platforms
    installNotificationConfig();
}

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
