#include "groupchatroom.h"

#include "src/core/core.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/persistence/settings.h"

GroupChatroom::GroupChatroom(Group* group)
    : group{group}
{
}

Contact* GroupChatroom::getContact()
{
    return group;
}

Group* GroupChatroom::getGroup()
{
    return group;
}

bool GroupChatroom::hasNewMessage() const
{
    return group->getEventFlag();
}

void GroupChatroom::resetEventFlags()
{
    group->setEventFlag(false);
    group->setMentionedFlag(false);
}

bool GroupChatroom::friendExists(const ToxPk& pk)
{
    return FriendList::findFriend(pk) != nullptr;
}

void GroupChatroom::inviteFriend(const ToxPk& pk)
{
    const Friend* frnd = FriendList::findFriend(pk);
    const auto friendId = frnd->getId();
    const auto groupId = group->getId();
    const auto canInvite = frnd->getStatus() != Status::Offline;

    if (canInvite) {
        Core::getInstance()->groupInviteFriend(friendId, groupId);
    }
}
