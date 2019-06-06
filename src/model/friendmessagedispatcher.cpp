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

#include "friendmessagedispatcher.h"
#include "src/persistence/settings.h"


namespace {

/**
 * @brief Sends message to friend using messageSender
 * @param[in] messageSender
 * @param[in] f
 * @param[in] message
 * @param[out] receipt
 */
bool sendMessageToCore(ICoreFriendMessageSender& messageSender, const Friend& f,
                       const Message& message, ReceiptNum& receipt)
{
    uint32_t friendId = f.getId();

    auto sendFn = message.isAction ? std::mem_fn(&ICoreFriendMessageSender::sendAction)
                                   : std::mem_fn(&ICoreFriendMessageSender::sendMessage);

    return sendFn(messageSender, friendId, message.content, receipt);
}
} // namespace

FriendMessageDispatcher::FriendMessageDispatcher(Friend& f_, MessageProcessor processor_,
                                                 ICoreFriendMessageSender& messageSender_)
    : f(f_)
    , messageSender(messageSender_)
    , offlineMsgEngine(&f_, &messageSender_)
    , processor(std::move(processor_))
{
    connect(&f, &Friend::statusChanged, this, &FriendMessageDispatcher::onFriendStatusChange);
}

/**
 * @see IMessageSender::sendMessage
 */
std::pair<DispatchedMessageId, DispatchedMessageId>
FriendMessageDispatcher::sendMessage(bool isAction, const QString& content)
{
    const auto firstId = nextMessageId;
    auto lastId = nextMessageId;
    for (const auto& message : processor.processOutgoingMessage(isAction, content)) {
        auto messageId = nextMessageId++;
        lastId = messageId;
        auto onOfflineMsgComplete = [this, messageId] { emit this->messageComplete(messageId); };

        ReceiptNum receipt;

        bool messageSent = false;

        if (f.isOnline()) {
            messageSent = sendMessageToCore(messageSender, f, message, receipt);
        }

        if (!messageSent) {
            offlineMsgEngine.addUnsentMessage(message, onOfflineMsgComplete);
        } else {
            offlineMsgEngine.addSentMessage(receipt, message, onOfflineMsgComplete);
        }

        emit this->messageSent(messageId, message);
    }
    return std::make_pair(firstId, lastId);
}

/**
 * @brief Handles received message from toxcore
 * @param[in] isAction True if action message
 * @param[in] content Unprocessed toxcore message
 */
void FriendMessageDispatcher::onMessageReceived(bool isAction, const QString& content)
{
    emit this->messageReceived(f.getPublicKey(), processor.processIncomingMessage(isAction, content));
}

/**
 * @brief Handles received receipt from toxcore
 * @param[in] receipt receipt id
 */
void FriendMessageDispatcher::onReceiptReceived(ReceiptNum receipt)
{
    offlineMsgEngine.onReceiptReceived(receipt);
}

/**
 * @brief Handles status change for friend
 * @note Parameters just to fit slot api
 */
void FriendMessageDispatcher::onFriendStatusChange(const ToxPk&, Status::Status)
{
    if (f.isOnline()) {
        offlineMsgEngine.deliverOfflineMsgs();
    }
}

/**
 * @brief Clears all currently outgoing messages
 */
void FriendMessageDispatcher::clearOutgoingMessages()
{
    offlineMsgEngine.removeAllMessages();
}
