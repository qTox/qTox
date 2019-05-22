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

void DesktopNotify::createNotification(const QString& title, const QString &text, Snore::Icon &icon)
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    Snore::Notification notify{snoreApp, Snore::Alert(), title, text, icon};

    notifyCore.broadcastNotification(notify);
}

void DesktopNotify::notifyMessage(const QString title, const QString message)
{
    createNotification(title, message, snoreIcon);
}

void DesktopNotify::notifyMessagePixmap(const QString title, const QString message, QPixmap avatar)
{
    Snore::Icon new_icon(avatar);
    createNotification(title, message, new_icon);
}

void DesktopNotify::notifyMessageSimple(const MessageType type)
{
    QString message;
    switch (type) {
    case MSG_FRIEND: message = tr("New message"); break;
    case MSG_FRIEND_FILE: message = tr("Incoming file transfer"); break;
    case MSG_FRIEND_REQUEST: message = tr("Friend request received"); break;
    case MSG_GROUP: message = tr("New group message"); break;
    case MSG_GROUP_INVITE: message = tr("Group invite received"); break;
    default: break;
    }

    createNotification(message, {}, snoreIcon);
}
