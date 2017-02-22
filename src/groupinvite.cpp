#include "groupinvite.h"

GroupInvite::GroupInvite(const int32_t FriendId, const uint8_t Type, const QByteArray& Invite)
    : friendId(FriendId), type(Type), invite(Invite), date(QDateTime::currentDateTime())
{
}

bool GroupInvite::operator ==(const GroupInvite& other) const
{
    return  friendId == other.friendId &&
            type == other.type &&
            invite == other.invite &&
            date == other.date;
}

int32_t GroupInvite::getFriendId() const
{
    return friendId;
}

uint8_t GroupInvite::getType() const
{
    return type;
}

QByteArray GroupInvite::getInvite() const
{
    return invite;
}

QDateTime GroupInvite::getInviteDate() const
{
    return date;
}
