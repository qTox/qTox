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

    enum class MessageType {
        FRIEND,
        FRIEND_FILE,
        FRIEND_REQUEST,
        GROUP,
        GROUP_INVITE
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
