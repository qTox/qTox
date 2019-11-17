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

FriendMessageDispatcher::FriendMessageDispatcher(Friend& f_, MessageProcessor processor_,
                                                 ICoreFriendMessageSender& messageSender_,
                                                 ICoreExtPacketAllocator& coreExtPacketAllocator_)
    : f(f_)
    , messageSender(messageSender_)
    , processor(std::move(processor_))
    , coreExtPacketAllocator(coreExtPacketAllocator_)
{
    connect(&f, &Friend::statusChanged, this, &FriendMessageDispatcher::onFriendStatusChange);
}

/**
 * @see IMessageDispatcher::sendMessage
 */
std::pair<DispatchedMessageId, DispatchedMessageId>
FriendMessageDispatcher::sendMessage(bool isAction, const QString& content)
{
    const auto firstId = nextMessageId;
    auto lastId = nextMessageId;
    for (const auto& message : processor.processOutgoingMessage(isAction, content, f.getSupportedExtensions())) {
        auto messageId = nextMessageId++;
        lastId = messageId;

        auto onOfflineMsgComplete = getCompletionFn(messageId);
        sendProcessedMessage(message, onOfflineMsgComplete);

        emit this->messageSent(messageId, message);
    }
    return std::make_pair(firstId, lastId);
}

/**
 * @see IMessageDispatcher::sendExtendedMessage
 */
DispatchedMessageId FriendMessageDispatcher::sendExtendedMessage(const QString& content, ExtensionSet extensions)
{
    auto messageId = nextMessageId++;

    auto messages = processor.processOutgoingMessage(false, content, extensions);
    assert(messages.size() == 1);

    auto onOfflineMsgComplete = getCompletionFn(messageId);
    sendProcessedMessage(messages[0], onOfflineMsgComplete);

    emit this->messageSent(messageId, messages[0]);

    return messageId;
}

/**
 * @brief Handles received message from toxcore
 * @param[in] isAction True if action message
 * @param[in] content Unprocessed toxcore message
 */
void FriendMessageDispatcher::onMessageReceived(bool isAction, const QString& content)
{
    emit this->messageReceived(f.getPublicKey(), processor.processIncomingCoreMessage(isAction, content));
}

/**
 * @brief Handles received receipt from toxcore
 * @param[in] receipt receipt id
 */
void FriendMessageDispatcher::onReceiptReceived(ReceiptNum receipt)
{
    offlineMsgEngine.onReceiptReceived(receipt);
}

void FriendMessageDispatcher::onExtMessageReceived(const QString& content)
{
    auto message = processor.processIncomingExtMessage(content);
    emit this->messageReceived(f.getPublicKey(), message);
}

void FriendMessageDispatcher::onExtReceiptReceived(uint64_t receiptId)
{
    offlineMsgEngine.onExtendedReceiptReceived(ExtendedReceiptNum(receiptId));
}

/**
 * @brief Handles status change for friend
 * @note Parameters just to fit slot api
 */
void FriendMessageDispatcher::onFriendStatusChange(const ToxPk&, Status::Status)
{
    // Note that this may end up double sending a message on a transition from online -> busy.
    // This was existing behavior and should be fixed in another PR
    if (f.isOnline()) {
        auto messagesToResend = offlineMsgEngine.removeAllMessages();
        for (auto const& message : messagesToResend) {
            sendProcessedMessage(message.message, message.callback);
        }
    }
}

/**
 * @brief Clears all currently outgoing messages
 */
void FriendMessageDispatcher::clearOutgoingMessages()
{
    offlineMsgEngine.removeAllMessages();
}


void FriendMessageDispatcher::sendProcessedMessage(Message const& message, OfflineMsgEngine::CompletionFn onOfflineMsgComplete)
{
    if (!f.isOnline()) {
        offlineMsgEngine.addUnsentMessage(message, onOfflineMsgComplete);
        return;
    }

    if (message.extensionSet[ExtensionType::messages]) {
        sendExtendedProcessedMessage(message, onOfflineMsgComplete);
    } else {
        sendCoreProcessedMessage(message, onOfflineMsgComplete);
    }
}



void FriendMessageDispatcher::sendExtendedProcessedMessage(Message const& message, OfflineMsgEngine::CompletionFn onOfflineMsgComplete)
{
    assert(!message.isAction); // Actions not supported with extensions

    if ((f.getSupportedExtensions() & message.extensionSet) != message.extensionSet) {
        onOfflineMsgComplete(false);
        return;
    }

    auto receipt = ExtendedReceiptNum();

    auto packet = coreExtPacketAllocator.getPacket(f.getId());

    if (message.extensionSet[ExtensionType::messages]) {
        receipt.get() = packet->addExtendedMessage(message.content);
    }

    const auto messageSent = packet->send();

    if (messageSent) {
        offlineMsgEngine.addSentExtendedMessage(receipt, message, onOfflineMsgComplete);
    } else {
        offlineMsgEngine.addUnsentMessage(message, onOfflineMsgComplete);
    }
}

void FriendMessageDispatcher::sendCoreProcessedMessage(Message const& message, OfflineMsgEngine::CompletionFn onOfflineMsgComplete)
{
    auto receipt = ReceiptNum();

    uint32_t friendId = f.getId();

    auto sendFn = message.isAction ? std::mem_fn(&ICoreFriendMessageSender::sendAction)
                                   : std::mem_fn(&ICoreFriendMessageSender::sendMessage);

    const auto messageSent = sendFn(messageSender, friendId, message.content, receipt);

    if (messageSent) {
        offlineMsgEngine.addSentCoreMessage(receipt, message, onOfflineMsgComplete);
    } else {
        offlineMsgEngine.addUnsentMessage(message, onOfflineMsgComplete);
    }
}

OfflineMsgEngine::CompletionFn FriendMessageDispatcher::getCompletionFn(DispatchedMessageId messageId)
{
    return [this, messageId] (bool success) {
        if (success) {
            emit this->messageComplete(messageId);
        } else {
            // For now we know the only reason we can fail after giving to the
            // offline message engine is due to a reduced extension set
            emit this->messageBroken(messageId, BrokenMessageReason::unsupportedExtensions);
        }
    };
}
