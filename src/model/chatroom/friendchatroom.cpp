#include "src/model/chatroom/friendchatroom.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/persistence/settings.h"
#include "src/widget/contentdialog.h"

FriendChatroom::FriendChatroom(Friend* frnd)
    : frnd{frnd}
{
}

Friend* FriendChatroom::getFriend()
{
    return frnd;
}

void FriendChatroom::setActive(bool _active)
{
    if (active != _active) {
        active = _active;
        emit activeChanged(active);
    }
}

bool FriendChatroom::canBeInvited() const
{
    return frnd->getStatus() != Status::Offline;
}

int FriendChatroom::getCircleId() const
{
    return Settings::getInstance().getFriendCircleID(frnd->getPublicKey());
}

QString FriendChatroom::getCircleName() const
{
    const auto circleId = getCircleId();
    return Settings::getInstance().getCircleName(circleId);
}

void FriendChatroom::inviteToNewGroup()
{
    auto core = Core::getInstance();
    const auto friendId = frnd->getId();
    const auto groupId = core->createGroup();
    core->groupInviteFriend(friendId, groupId);
}

QString FriendChatroom::getAutoAcceptDir() const
{
    const auto pk = frnd->getPublicKey();
    return Settings::getInstance().getAutoAcceptDir(pk);
}

void FriendChatroom::setAutoAcceptDir(const QString& dir)
{
    const auto pk = frnd->getPublicKey();
    Settings::getInstance().setAutoAcceptDir(pk, dir);
}

void FriendChatroom::disableAutoAccept()
{
    setAutoAcceptDir(QString{});
}

bool FriendChatroom::autoAcceptEnabled() const
{
    return getAutoAcceptDir().isEmpty();
}

void FriendChatroom::inviteFriend(uint32_t friendId, const Group* group)
{
    Core::getInstance()->groupInviteFriend(friendId, group->getId());
}
