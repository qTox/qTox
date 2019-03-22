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

Snore::Notification DesktopNotify::createNotification(const QString& title, const QString& text)
{
    return Snore::Notification(snoreApp, Snore::Alert(), title, text, snoreIcon);
}

void DesktopNotify::notifyGroupMessage()
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    const QString text{};
    const QString title = tr("New group message received");
    Snore::Notification groupMessage = createNotification(title, text);
    notifyCore.broadcastNotification(groupMessage);
}

void DesktopNotify::notifyFriendRequest()
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    const QString title = tr("New friend request received");
    const QString text{};
    Snore::Notification friendRequest = createNotification(title, text);
    notifyCore.broadcastNotification(friendRequest);
}

void DesktopNotify::notifyGroupInvite()
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    const QString title = tr("New group invite received");
    const QString text{};
    Snore::Notification groupInvite = createNotification(title, text);
    notifyCore.broadcastNotification(groupInvite);
}

void DesktopNotify::notifyFriendMessage()
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    const QString title = tr("New message received");
    const QString text{};
    Snore::Notification friendMessage = createNotification(title, text);
    notifyCore.broadcastNotification(friendMessage);
}
