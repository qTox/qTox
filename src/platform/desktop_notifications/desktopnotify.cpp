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

#include <libsnore/snore.h>

#include <QDebug>

DesktopNotify::DesktopNotify()
    : notifyCore{Snore::SnoreCore::instance()}
    , snoreIcon{":/img/icons/qtox.svg"}
{

    notifyCore.loadPlugins(Snore::SnorePlugin::Backend);
    qDebug() << "primary notification backend:" << notifyCore.primaryNotificationBackend();

    snoreApp = Snore::Application("qTox", snoreIcon);

    notifyCore.registerApplication(snoreApp);
}

void DesktopNotify::createNotification(const QString& title, const QString& text, Snore::Icon& icon)
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    Snore::Notification notify{snoreApp, Snore::Alert(), title, text, icon};

    notifyCore.broadcastNotification(notify);
}

void DesktopNotify::notifyMessage(const QString& title, const QString& message)
{
    createNotification(title, message, snoreIcon);
}

void DesktopNotify::notifyMessagePixmap(const QString& title, const QString& message, QPixmap avatar)
{
    Snore::Icon new_icon(avatar);
    createNotification(title, message, new_icon);
}

void DesktopNotify::notifyMessageSimple(const MessageType type)
{
    QString message;
    switch (type) {
    case MessageType::FRIEND: message = tr("New message"); break;
    case MessageType::FRIEND_FILE: message = tr("Incoming file transfer"); break;
    case MessageType::FRIEND_REQUEST: message = tr("Friend request received"); break;
    case MessageType::GROUP: message = tr("New group message"); break;
    case MessageType::GROUP_INVITE: message = tr("Group invite received"); break;
    default: break;
    }

    createNotification(message, {}, snoreIcon);
}
#endif
