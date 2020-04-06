#pragma once


#include "notificationdata.h"
#include "friend.h"
#include "group.h"

#include "src/persistence/inotificationsettings.h"
#include "src/persistence/profile.h"

#include <QObject>
#include <QHash>

class NotificationGenerator : public QObject
{
public:
    NotificationGenerator(
        INotificationSettings const& notificationSettings,
        // Optional profile input to lookup avatars. Avatar lookup is not
        // currently mockable so we allow profile to be nullptr for unit
        // testing
        Profile* profile);

    NotificationData friendMessageNotification(const Friend* f, const QString& message);
    NotificationData groupMessageNotification(const Group* g, const ToxPk& sender, const QString& message);
    NotificationData fileTransferNotification(const Friend* f, const QString& filename, size_t fileSize);
    NotificationData groupInvitationNotification(const Friend* from);
    NotificationData friendRequestNotification(const ToxPk& sender, const QString& message);

public slots:
    void onNotificationActivated();

private:
    INotificationSettings const& notificationSettings;
    Profile* profile;
    QHash<const Friend*, size_t> friendNotifications;
    QHash<const Group*, size_t> groupNotifications;
};
