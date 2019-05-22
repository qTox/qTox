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

