/*
    Copyright © 2019 by The qTox Project Contributors

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

#ifndef IMESSAGE_DISPATCHER_H
#define IMESSAGE_DISPATCHER_H

#include "src/model/friend.h"
#include "src/model/message.h"

#include <QObject>
#include <QString>

#include <cstdint>

using DispatchedMessageId = NamedType<size_t, struct SentMessageIdTag, Orderable, Incrementable>;
Q_DECLARE_METATYPE(DispatchedMessageId)

class IMessageDispatcher : public QObject
{
    Q_OBJECT
public:
    virtual ~IMessageDispatcher() = default;

    /**
     * @brief Sends message to associated chat
     * @param[in] isAction True if is action message
     * @param[in] content Message content
     * @return Pair of first and last dispatched message IDs
     */
    virtual std::pair<DispatchedMessageId, DispatchedMessageId>
    sendMessage(bool isAction, const QString& content) = 0;
signals:
    /**
     * @brief Emitted when a message is received and processed
     */
    void messageReceived(const ToxPk& sender, const Message& message);

    /**
     * @brief Emitted when a message is processed and sent
     * @param id message id for completion
     * @param message sent message
     */
    void messageSent(DispatchedMessageId id, const Message& message);

    /**
     * @brief Emitted when a receiver report is received from the associated chat
     * @param id Id of message that is completed
     */
    void messageComplete(DispatchedMessageId id);
};

#endif /* IMESSAGE_DISPATCHER_H */
