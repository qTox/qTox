#include "groupchatroom.h"

#include "src/core/core.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"
#include "src/model/dialogs/idialogsmanager.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/persistence/settings.h"

GroupChatroom::GroupChatroom(Group* group, IDialogsManager* dialogsManager)
    : group{group}
    , dialogsManager{dialogsManager}
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
    const auto canInvite = frnd->isOnline();

    if (canInvite) {
        Core::getInstance()->groupInviteFriend(friendId, groupId);
    }
}

bool GroupChatroom::possibleToOpenInNewWindow() const
{
    const auto groupId = group->getPersistentId();
    const auto dialogs = dialogsManager->getGroupDialogs(groupId);
    return !dialogs || dialogs->chatroomCount() > 1;
}

bool GroupChatroom::canBeRemovedFromWindow() const
{
    const auto groupId = group->getPersistentId();
    const auto dialogs = dialogsManager->getGroupDialogs(groupId);
    return dialogs && dialogs->hasContact(groupId);
}

void GroupChatroom::removeGroupFromDialogs()
{
    const auto groupId = group->getPersistentId();
    auto dialogs = dialogsManager->getGroupDialogs(groupId);
    dialogs->removeGroup(groupId);
}
