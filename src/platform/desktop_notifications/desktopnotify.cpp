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

void DesktopNotify::createNotification(const QString& title)
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    Snore::Notification notify{snoreApp, Snore::Alert(), title, {}, snoreIcon};
    notifyCore.broadcastNotification(notify);
}

void DesktopNotify::notifyGroupMessage()
{
    const QString title = tr("New group message received");
    createNotification(title);
}

void DesktopNotify::notifyFriendRequest()
{
    const QString title = tr("New friend request received");
    createNotification(title);
}

void DesktopNotify::notifyGroupInvite()
{
    const QString title = tr("New group invite received");
    createNotification(title);
}

void DesktopNotify::notifyFriendMessage()
{
    const QString title = tr("New message received");
    createNotification(title);
}
