#ifndef DESKTOPNOTIFY_H
#define DESKTOPNOTIFY_H

#include <libsnore/snore.h>

#include <QObject>
#include <memory>

class DesktopNotify : public QObject
{
    Q_OBJECT
public:
    DesktopNotify();

public slots:
    void notifyFriendMessage();
    void notifyGroupMessage();
    void notifyFriendRequest();
    void notifyGroupInvite();

private:
    Snore::Notification createNotification(const QString& title, const QString& text);

private:
    Snore::SnoreCore& notifyCore;
    Snore::Application snoreApp;
    Snore::Icon snoreIcon;
};

#endif // DESKTOPNOTIFY_H
