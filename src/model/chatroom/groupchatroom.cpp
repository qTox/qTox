/*
    Copyright © 2014-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

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
