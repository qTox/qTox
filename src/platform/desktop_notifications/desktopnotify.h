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
    using NotificationPtr = std::unique_ptr<Snore::Notification>;

    NotificationPtr createNotification(const QString& title, const QString& text,
                                       Snore::Notification* old = nullptr);

private:
    Snore::SnoreCore& notifyCore;
    NotificationPtr groupInvite = {nullptr};
    NotificationPtr groupMessage = {nullptr};
    NotificationPtr friendRequest = {nullptr};
    NotificationPtr friendMessage = {nullptr};
    Snore::Application snoreApp;
    Snore::Icon snoreIcon;
};

#endif // DESKTOPNOTIFY_H
