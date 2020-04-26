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

OfflineMsgEngine::OfflineMsgEngine(Friend* frnd, ICoreFriendMessageSender* messageSender)
    : mutex(QMutex::Recursive)
    , f(frnd)
    , messageSender(messageSender)
{}

/**
* @brief Notification that the message is now delivered.
*
* @param[in] receipt   Toxcore message ID which the receipt is for.
*/
void OfflineMsgEngine::onReceiptReceived(ReceiptNum receipt)
{
    QMutexLocker ml(&mutex);
    if (receivedReceipts.contains(receipt)) {
        qWarning() << "Receievd duplicate receipt" << receipt.get() << "from friend" << f->getId();
        return;
    }
    receivedReceipts.append(receipt);
    checkForCompleteMessages(receipt);
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
    unsentMessages.append(OfflineMessage{message, std::chrono::steady_clock::now(), completionCallback});
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
void OfflineMsgEngine::addSentMessage(ReceiptNum receipt, Message const& message,
                                      CompletionFn completionCallback)
{
    QMutexLocker ml(&mutex);
    assert(!sentMessages.contains(receipt));
    sentMessages.insert(receipt, {message, std::chrono::steady_clock::now(), completionCallback});
    checkForCompleteMessages(receipt);
}

/**
* @brief Deliver all messages, used when a friend comes online.
*/
void OfflineMsgEngine::deliverOfflineMsgs()
{
    QMutexLocker ml(&mutex);

    if (!Status::isOnline(f->getStatus())) {
        return;
    }

    if (sentMessages.empty() && unsentMessages.empty()) {
        return;
    }

    QVector<OfflineMessage> messages = sentMessages.values().toVector() + unsentMessages;
    // order messages by authorship time to resend in same order as they were written
    std::sort(messages.begin(), messages.end(), [](const OfflineMessage& lhs, const OfflineMessage& rhs) {
        return lhs.authorshipTime < rhs.authorshipTime;
    });
    removeAllMessages();

    for (const auto& message : messages) {
        QString messageText = message.message.content;
        ReceiptNum receipt;
        bool messageSent{false};
        if (message.message.isAction) {
            messageSent = messageSender->sendAction(f->getId(), messageText, receipt);
        } else {
            messageSent = messageSender->sendMessage(f->getId(), messageText, receipt);
        }
        if (messageSent) {
            addSentMessage(receipt, message.message, message.completionFn);
        } else {
            qCritical() << "deliverOfflineMsgs failed to send message";
            addUnsentMessage(message.message, message.completionFn);
        }
    }
}

/**
* @brief Removes all messages which are being tracked.
*/
void OfflineMsgEngine::removeAllMessages()
{
    QMutexLocker ml(&mutex);
    receivedReceipts.clear();
    sentMessages.clear();
    unsentMessages.clear();
}

void OfflineMsgEngine::completeMessage(QMap<ReceiptNum, OfflineMessage>::iterator msgIt)
{
    msgIt->completionFn();
    receivedReceipts.removeOne(msgIt.key());
    sentMessages.erase(msgIt);
}

void OfflineMsgEngine::checkForCompleteMessages(ReceiptNum receipt)
{
    auto msgIt = sentMessages.find(receipt);
    const bool receiptReceived = receivedReceipts.contains(receipt);
    if (!receiptReceived || msgIt == sentMessages.end()) {
        return;
    }
    completeMessage(msgIt);
}
