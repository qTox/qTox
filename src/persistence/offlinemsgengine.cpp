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

#include "offlinemsgengine.h"
#include "src/core/core.h"
#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/model/status.h"
#include <QMutexLocker>
#include <QTimer>
#include <QCoreApplication>
#include <chrono>

/**
* @brief Notification that the message is now delivered.
*
* @param[in] receipt   Toxcore message ID which the receipt is for.
*/
void OfflineMsgEngine::onReceiptReceived(ReceiptNum receipt)
{
    QMutexLocker ml(&mutex);
    receiptResolver.notifyReceiptReceived(receipt);
}

void OfflineMsgEngine::onExtendedReceiptReceived(ExtendedReceiptNum receipt)
{
    QMutexLocker ml(&mutex);
    extendedReceiptResolver.notifyReceiptReceived(receipt);
}

/**
* @brief Add a message which has been saved to history, but not sent yet to the peer.
*
* OfflineMsgEngine will send this message once the friend becomes online again, then track its
* receipt, updating history and chatlog once received.
*
* @param[in] messageID   database RowId of the message, used to eventually mark messages as received in history
* @param[in] msg         chat message line in the chatlog, used to eventually set the message's receieved timestamp
*/
void OfflineMsgEngine::addUnsentMessage(Message const& message, CompletionFn completionCallback)
{
    QMutexLocker ml(&mutex);
    unsentMessages.push_back(OfflineMessage{message, std::chrono::steady_clock::now(), completionCallback});
}

/**
* @brief Add a message which has been saved to history, and which has been sent to the peer.
*
* OfflineMsgEngine will track this message's receipt. If the friend goes offline then comes back before the receipt
* is received, OfflineMsgEngine will also resend the message, updating history and chatlog once received.
*
* @param[in] receipt     the toxcore message ID, corresponding to expected receipt ID
* @param[in] messageID   database RowId of the message, used to eventually mark messages as received in history
* @param[in] msg         chat message line in the chatlog, used to eventually set the message's receieved timestamp
*/
void OfflineMsgEngine::addSentCoreMessage(ReceiptNum receipt, Message const& message,
                                      CompletionFn completionCallback)
{
    QMutexLocker ml(&mutex);
    receiptResolver.notifyMessageSent(receipt, {message, std::chrono::steady_clock::now(), completionCallback});
}

void OfflineMsgEngine::addSentExtendedMessage(ExtendedReceiptNum receipt, Message const& message,
                                      CompletionFn completionCallback)
{
    QMutexLocker ml(&mutex);
    extendedReceiptResolver.notifyMessageSent(receipt, {message, std::chrono::steady_clock::now(), completionCallback});
}

/**
* @brief Removes all messages which are being tracked.
*/
std::vector<OfflineMsgEngine::RemovedMessage> OfflineMsgEngine::removeAllMessages()
{
    QMutexLocker ml(&mutex);
    auto messages = receiptResolver.clear();
    auto extendedMessages = extendedReceiptResolver.clear();

    messages.insert(
        messages.end(),
        std::make_move_iterator(extendedMessages.begin()),
        std::make_move_iterator(extendedMessages.end()));

    messages.insert(
        messages.end(),
        std::make_move_iterator(unsentMessages.begin()),
        std::make_move_iterator(unsentMessages.end()));

    unsentMessages.clear();

    std::sort(messages.begin(), messages.end(), [] (const OfflineMessage& a, const OfflineMessage& b) {
        return a.authorshipTime < b.authorshipTime;
    });

    auto ret = std::vector<RemovedMessage>();
    ret.reserve(messages.size());

    std::transform(messages.begin(), messages.end(), std::back_inserter(ret), [](const OfflineMessage& msg) {
        return RemovedMessage{msg.message, msg.completionFn};
    });

    return ret;
}
