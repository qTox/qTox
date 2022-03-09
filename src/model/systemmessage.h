/*
    Copyright © 2015-2021 by The qTox Project Contributors

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

#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>

#include <array>

enum class SystemMessageType
{
    // DO NOT CHANGE ORDER
    // These values are saved directly to the DB and read back, changing the
    // order will break persistence!
    fileSendFailed = 0,
    userJoinedGroup,
    userLeftGroup,
    peerNameChanged,
    peerStateChange,
    titleChanged,
    cleared,
    unexpectedCallEnd,
    outgoingCall,
    incomingCall,
    callEnd,
    messageSendFailed,
    selfJoinedGroup,
    selfLeftGroup,
};

struct SystemMessage
{
    using Args = std::array<QString, 4>;
    SystemMessageType messageType;
    QDateTime timestamp;
    Args args;

    QString toString() const
    {
        switch (messageType) {
        case SystemMessageType::fileSendFailed:
            return QObject::tr("Failed to send file \"%1\"").arg(args[0]);
        case SystemMessageType::userJoinedGroup:
            return QObject::tr("%1 has joined the group").arg(args[0]);
        case SystemMessageType::userLeftGroup:
            return QObject::tr("%1 has left the group").arg(args[0]);
        case SystemMessageType::peerNameChanged:
            return QObject::tr("%1 is now known as %2").arg(args[0]).arg(args[1]);
        case SystemMessageType::titleChanged:
            return QObject::tr("%1 has set the title to %2").arg(args[0]).arg(args[1]);
        case SystemMessageType::cleared:
            return QObject::tr("Cleared");
        case SystemMessageType::unexpectedCallEnd:
            return QObject::tr("Call with %1 ended unexpectedly. %2").arg(args[0]).arg(args[1]);
        case SystemMessageType::callEnd:
            return QObject::tr("Call with %1 ended. %2").arg(args[0]).arg(args[1]);
        case SystemMessageType::peerStateChange:
            return QObject::tr("%1 is now %2", "e.g. \"Dubslow is now online\"").arg(args[0]).arg(args[1]);
        case SystemMessageType::outgoingCall:
            return QObject::tr("Calling %1").arg(args[0]);
        case SystemMessageType::incomingCall:
            return QObject::tr("%1 calling").arg(args[0]);
        case SystemMessageType::messageSendFailed:
            return QObject::tr("Message failed to send");
        case SystemMessageType::selfJoinedGroup:
            return QObject::tr("You have joined the group");
        case SystemMessageType::selfLeftGroup:
            return QObject::tr("You have left the group");
        }
        return {};
    }
};
