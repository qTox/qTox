/*
    Copyright © 2015-2019 by The qTox Project Contributors

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


#include "avatarbroadcaster.h"
#include "src/core/core.h"
#include "src/core/corefile.h"
#include "src/model/status.h"
#include <QDebug>
#include <QObject>

/**
 * @class AvatarBroadcaster
 *
 * Takes care of broadcasting avatar changes to our friends in a smart way
 * Cache a copy of our current avatar and friends who have received it
 * so we don't spam avatar transfers to a friend who already has it.
 */

AvatarBroadcaster::AvatarBroadcaster(Core& _core)
    : core{_core}
{}

/**
 * @brief Set our current avatar.
 * @param data Byte array on avater.
 */
void AvatarBroadcaster::setAvatar(QByteArray data)
{
    if (avatarData == data) {
        return;
    }

    avatarData = data;
    friendsSentTo.clear();

    QVector<uint32_t> friends = core.getFriendList();
    for (uint32_t friendId : friends) {
        sendAvatarTo(friendId);
    }
}

/**
 * @brief Send our current avatar to this friend, if not already sent
 * @param friendId Id of friend to send avatar.
 */
void AvatarBroadcaster::sendAvatarTo(uint32_t friendId)
{
    if (friendsSentTo.contains(friendId) && friendsSentTo[friendId]) {
        return;
    }

    if (!core.isFriendOnline(friendId)) {
        return;
    }

    CoreFile* coreFile = core.getCoreFile();
    coreFile->sendAvatarFile(friendId, avatarData);
    friendsSentTo[friendId] = true;
}

/**
 * @brief Setup auto broadcast sending avatar.
 * @param state If true, we automatically broadcast our avatar to friends when they come online.
 */
void AvatarBroadcaster::enableAutoBroadcast(bool state)
{
    disconnect(&core, nullptr, this, nullptr);
    if (state) {
        connect(&core, &Core::friendStatusChanged,
                [=](uint32_t friendId, Status::Status) { sendAvatarTo(friendId); });
    }
}
