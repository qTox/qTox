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

DesktopNotify::NotificationPtr DesktopNotify::createNotification(const QString& title,
                                                                 const QString& text,
                                                                 Snore::Notification* old)
{
    if (old == nullptr) {
        return NotificationPtr(
            new Snore::Notification(snoreApp, Snore::Alert(), title, text, snoreIcon));
    } else {
        return NotificationPtr(new Snore::Notification(*old, title, text, snoreIcon));
    }
}

void DesktopNotify::notifyGroupMessage()
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    const QString text{};
    const QString title = tr("New group message received");
    NotificationPtr newNote = createNotification(title, text);
    if (!newNote) {
        qDebug() << "Failed to allocate group message notification";
        return;
    }
    groupInvite = std::move(newNote);
}

void DesktopNotify::notifyFriendRequest()
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    const QString title = tr("New friend request received");
    const QString text{};
    NotificationPtr newNote = createNotification(title, text);
    if (!newNote) {
        qDebug() << "Failed to allocate friend message notification";
        return;
    }
    friendMessage = std::move(newNote);
    notifyCore.broadcastNotification(*friendMessage);
}

void DesktopNotify::notifyGroupInvite()
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    const QString title = tr("New group invite received");
    const QString text{};
    NotificationPtr newNote = createNotification(title, text);
    if (!newNote) {
        qDebug() << "Failed to allocate friend message notification";
        return;
    }
    friendMessage = std::move(newNote);
    notifyCore.broadcastNotification(*friendMessage);
}

void DesktopNotify::notifyFriendMessage()
{
    const Settings& s = Settings::getInstance();
    if(!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    const QString title = tr("New message received");
    const QString text{};
    NotificationPtr newNote = createNotification(title, text);
    if (!newNote) {
        qDebug() << "Failed to allocate friend message notification";
        return;
    }
    friendMessage = std::move(newNote);
    notifyCore.broadcastNotification(*friendMessage);
}
