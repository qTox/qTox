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

    enum MessageType {
        MSG_FRIEND = 0,
        MSG_FRIEND_FILE,
        MSG_FRIEND_REQUEST,
        MSG_GROUP,
        MSG_GROUP_INVITE
    };

public slots:
    void notifyMessage(const QString title, const QString message);
    void notifyMessagePixmap(const QString title, const QString message, QPixmap avatar);
    void notifyMessageSimple(const MessageType type);

private:
    void createNotification(const QString& title, const QString &text, Snore::Icon &icon);

private:
    Snore::SnoreCore& notifyCore;
    Snore::Application snoreApp;
    Snore::Icon snoreIcon;
};

#endif // DESKTOPNOTIFY_H
