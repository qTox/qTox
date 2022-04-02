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

#include "src/grouplist.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/dialogs/idialogsmanager.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include "src/widget/contentdialog.h"

#include <QCollator>

namespace {

QString getShortName(const QString& name)
{
    constexpr auto MAX_NAME_LENGTH = 30;
    if (name.length() <= MAX_NAME_LENGTH) {
        return name;
    }

    return name.left(MAX_NAME_LENGTH).trimmed() + "…";
}

}

FriendChatroom::FriendChatroom(Friend* frnd_, IDialogsManager* dialogsManager_,
    Core& core_, Settings& settings_, GroupList& groupList_)
    : frnd{frnd_}
    , dialogsManager{dialogsManager_}
    , core{core_}
    , settings{settings_}
    , groupList{groupList_}
{
}

Friend* FriendChatroom::getFriend()
{
    return frnd;
}

Chat* FriendChatroom::getChat()
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
    return Status::isOnline(frnd->getStatus());
}

int FriendChatroom::getCircleId() const
{
    return settings.getFriendCircleID(frnd->getPublicKey());
}

QString FriendChatroom::getCircleName() const
{
    const auto circleId = getCircleId();
    return settings.getCircleName(circleId);
}

void FriendChatroom::inviteToNewGroup()
{
    const auto friendId = frnd->getId();
    const auto groupId = core.createGroup();
    core.groupInviteFriend(friendId, groupId);
}

QString FriendChatroom::getAutoAcceptDir() const
{
    const auto pk = frnd->getPublicKey();
    return settings.getAutoAcceptDir(pk);
}

void FriendChatroom::setAutoAcceptDir(const QString& dir)
{
    const auto pk = frnd->getPublicKey();
    settings.setAutoAcceptDir(pk, dir);
}

void FriendChatroom::disableAutoAccept()
{
    setAutoAcceptDir(QString{});
}

bool FriendChatroom::autoAcceptEnabled() const
{
    return getAutoAcceptDir().isEmpty();
}

void FriendChatroom::inviteFriend(const Group* group)
{
    const auto friendId = frnd->getId();
    const auto groupId = group->getId();
    core.groupInviteFriend(friendId, groupId);
}

QVector<GroupToDisplay> FriendChatroom::getGroups() const
{
    QVector<GroupToDisplay> groups;
    for (const auto group : groupList.getAllGroups()) {
        const auto name = getShortName(group->getName());
        const GroupToDisplay groupToDisplay = { name, group };
        groups.push_back(groupToDisplay);
    }

    return groups;
}

/**
 * @brief Return sorted list of circles exclude current circle.
 */
QVector<CircleToDisplay> FriendChatroom::getOtherCircles() const
{
    QVector<CircleToDisplay> circles;
    const auto currentCircleId = getCircleId();
    for (int i = 0; i < settings.getCircleCount(); ++i) {
        if (i == currentCircleId) {
            continue;
        }

        const auto name = getShortName(settings.getCircleName(i));
        const CircleToDisplay circle = { name, i };
        circles.push_back(circle);
    }

    std::sort(circles.begin(), circles.end(),
              [](const CircleToDisplay& a, const CircleToDisplay& b) -> bool {
                  QCollator collator;
                  collator.setNumericMode(true);
                  return collator.compare(a.name, b.name) < 0;
              });

    return circles;
}

void FriendChatroom::resetEventFlags()
{
    frnd->setEventFlag(false);
}

bool FriendChatroom::possibleToOpenInNewWindow() const
{
    const auto friendPk = frnd->getPublicKey();
    const auto dialogs = dialogsManager->getFriendDialogs(friendPk);
    return !dialogs || dialogs->chatroomCount() > 1;
}

bool FriendChatroom::canBeRemovedFromWindow() const
{
    const auto friendPk = frnd->getPublicKey();
    const auto dialogs = dialogsManager->getFriendDialogs(friendPk);
    return dialogs && dialogs->hasChat(friendPk);
}

bool FriendChatroom::friendCanBeRemoved() const
{
    const auto friendPk = frnd->getPublicKey();
    const auto dialogs = dialogsManager->getFriendDialogs(friendPk);
    return !dialogs || !dialogs->hasChat(friendPk);
}

void FriendChatroom::removeFriendFromDialogs()
{
    const auto friendPk = frnd->getPublicKey();
    auto dialogs = dialogsManager->getFriendDialogs(friendPk);
    dialogs->removeFriend(friendPk);
}
