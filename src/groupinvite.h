#ifndef GROUPINVITE_H
#define GROUPINVITE_H

#include <cstdint>
#include <QByteArray>
#include <QDateTime>

class GroupInvite
{
public:
    GroupInvite(const int32_t FriendId, const uint8_t Type, const QByteArray& Invite);
    bool operator ==(const GroupInvite& other) const;

    int32_t getFriendId() const;
    uint8_t getType() const;
    QByteArray getInvite() const;
    QDateTime getInviteDate() const;

private:
    int32_t friendId;
    uint8_t type;
    QByteArray invite;
    QDateTime date;
};

#endif // GROUPINVITE_H
