#ifndef AVATARBROADCASTER_H
#define AVATARBROADCASTER_H

#include <QByteArray>
#include <QMap>

/// Takes care of broadcasting avatar changes to our friends in a smart way
/// Cache a copy of our current avatar and friends who have received it
/// so we don't spam avatar transfers to a friend who already has it.
class AvatarBroadcaster
{
private:
    AvatarBroadcaster()=delete;

public:
    /// Set our current avatar
    static void setAvatar(QByteArray data);
    /// Send our current avatar to this friend, if not already sent
    static void sendAvatarTo(uint32_t friendId);
    /// If true, we automatically broadcast our avatar to friends when they come online
    static void enableAutoBroadcast(bool state = true);

private:
    static QByteArray avatarData;
    static QMap<uint32_t, bool> friendsSentTo;
};

#endif // AVATARBROADCASTER_H
