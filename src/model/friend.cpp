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


#include "friend.h"
#include "src/model/status.h"
#include "src/persistence/profile.h"
#include "src/widget/form/chatform.h"

#include <QDebug>

#include <memory>
#include <cassert>

Friend::Friend(uint32_t friendId_, const ToxPk& friendPk_, const QString& userAlias_, const QString& userName_)
    : userName{userName_}
    , userAlias{userAlias_}
    , friendPk{friendPk_}
    , friendId{friendId_}
    , hasNewEvents{false}
    , friendStatus{Status::Status::Offline}
    , isNegotiating{false}
{
    if (userName_.isEmpty()) {
        userName = friendPk.toString();
    }
}

/**
 * @brief Friend::setName sets a new username for the friend
 * @param _name new username, sets the public key if _name is empty
 */
void Friend::setName(const QString& _name)
{
    QString name = _name;
    if (name.isEmpty()) {
        name = friendPk.toString();
    }

    // save old displayed name to be able to compare for changes
    const auto oldDisplayed = getDisplayedName();
    if (userName == userAlias) {
        userAlias.clear(); // Because userAlias was set on name change before (issue #5013)
                           // we clear alias if equal to old name so that name change is visible.
                           // TODO: We should not modify alias on setName.
    }
    if (userName != name) {
        userName = name;
        emit nameChanged(friendPk, name);
    }

    const auto newDisplayed = getDisplayedName();
    if (oldDisplayed != newDisplayed) {
        emit displayedNameChanged(newDisplayed);
    }
}

/**
 * @brief Friend::setAlias sets the alias for the friend
 * @param alias new alias, removes it if set to an empty string
 */
void Friend::setAlias(const QString& alias)
{
    if (userAlias == alias) {
        return;
    }
    emit aliasChanged(friendPk, alias);

    // save old displayed name to be able to compare for changes
    const auto oldDisplayed = getDisplayedName();
    userAlias = alias;

    const auto newDisplayed = getDisplayedName();
    if (oldDisplayed != newDisplayed) {
        emit displayedNameChanged(newDisplayed);
    }
}

void Friend::setStatusMessage(const QString& message)
{
    if (statusMessage != message) {
        statusMessage = message;
        emit statusMessageChanged(friendPk, message);
    }
}

QString Friend::getStatusMessage() const
{
    return statusMessage;
}

/**
 * @brief Friend::getDisplayedName Gets the name that should be displayed for a user
 * @return a QString containing either alias, username or public key
 * @note This function and corresponding signal should be preferred over getting
 *       the name or alias directly.
 */
QString Friend::getDisplayedName() const
{
    if (userAlias.isEmpty()) {
        return userName;
    }

    return userAlias;
}

QString Friend::getDisplayedName(const ToxPk& contact) const
{
    std::ignore = contact;
    assert(contact == friendPk);
    return getDisplayedName();
}

bool Friend::hasAlias() const
{
    return !userAlias.isEmpty();
}

QString Friend::getUserName() const
{
    return userName;
}

const ToxPk& Friend::getPublicKey() const
{
    return friendPk;
}

uint32_t Friend::getId() const
{
    return friendId;
}

const ChatId& Friend::getPersistentId() const
{
    return friendPk;
}

void Friend::setEventFlag(bool flag)
{
    hasNewEvents = flag;
}

bool Friend::getEventFlag() const
{
    return hasNewEvents;
}

void Friend::setStatus(Status::Status s)
{
    // Internal status should never be negotiating. We only expose this externally through the use of isNegotiating
    assert(s != Status::Status::Negotiating);

    const bool wasOnline = Status::isOnline(getStatus());
    if (friendStatus == s) {
        return;
    }

    // When a friend goes online we want to give them some time to negotiate
    // extension support
    const auto startNegotating = friendStatus == Status::Status::Offline;

    if (startNegotating) {
        qDebug() << "Starting negotiation with friend " << friendId;
        isNegotiating = true;
    }

    friendStatus = s;
    const bool nowOnline = Status::isOnline(getStatus());

    const auto emitStatusChange = startNegotating || !isNegotiating;
    if (emitStatusChange) {
        const auto statusToEmit = isNegotiating ? Status::Status::Negotiating : friendStatus;
        emit statusChanged(friendPk, statusToEmit);
        if (wasOnline && !nowOnline) {
            emit onlineOfflineChanged(friendPk, false);
        } else if (!wasOnline && nowOnline) {
            emit onlineOfflineChanged(friendPk, true);
        }
    }
}

Status::Status Friend::getStatus() const
{
    return isNegotiating ? Status::Status::Negotiating : friendStatus;
}

void Friend::setExtendedMessageSupport(bool supported)
{
    supportedExtensions[ExtensionType::messages] = supported;
    emit extensionSupportChanged(supportedExtensions);

    // If all extensions are supported we can exit early
    if (supportedExtensions.all()) {
        onNegotiationComplete();
    }
}

ExtensionSet Friend::getSupportedExtensions() const
{
    return supportedExtensions;
}

void Friend::onNegotiationComplete() {
    if (!isNegotiating) {
        return;
    }

    qDebug() << "Negotiation complete for friend " << friendId;

    isNegotiating = false;
    emit statusChanged(friendPk, friendStatus);

    if (Status::isOnline(getStatus())) {
        emit onlineOfflineChanged(friendPk, true);
    }
}
