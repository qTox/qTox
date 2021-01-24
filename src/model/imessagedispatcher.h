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

#pragma once

#include "src/model/friend.h"
#include "src/model/message.h"
#include "src/model/brokenmessagereason.h"

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

    /**
     * @brief Sends message to associated chat ensuring that extensions are available
     * @param[in] content Message content
     * @param[in] extensions extensions required for given message
     * @return Pair of first and last dispatched message IDs
     * @note If the provided extensions are not supported the message will be flagged
     *       as broken
     */
    virtual std::pair<DispatchedMessageId, DispatchedMessageId>
    sendExtendedMessage(const QString& content, ExtensionSet extensions) = 0;

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

    void messageBroken(DispatchedMessageId id, BrokenMessageReason reason);
};
