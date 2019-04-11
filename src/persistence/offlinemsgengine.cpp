/*
    Copyright © 2014-2018 by The qTox Project Contributors

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
#include <QMutexLocker>
#include <QTimer>
#include <QCoreApplication>
#include <chrono>

OfflineMsgEngine::OfflineMsgEngine(Friend* frnd)
    : mutex(QMutex::Recursive)
    , f(frnd)
{
}

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
void OfflineMsgEngine::addSavedMessage(RowId messageID, ChatMessage::Ptr chatMessage)
{
    QMutexLocker ml(&mutex);
    assert([&](){
        for (const auto& message : unsentSavedMessages) {
            if (message.rowId == messageID) {
                return false;
            }
        }
        return true;
    }());
    unsentSavedMessages.append(Message{chatMessage, messageID, std::chrono::steady_clock::now()});
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
void OfflineMsgEngine::addSentSavedMessage(ReceiptNum receipt, RowId messageID, ChatMessage::Ptr chatMessage)
{
    QMutexLocker ml(&mutex);
    assert(!sentSavedMessages.contains(receipt));
    sentSavedMessages.insert(receipt, {chatMessage, messageID, std::chrono::steady_clock::now()});
    checkForCompleteMessages(receipt);
}

/**
* @brief Deliver all messages, used when a friend comes online.
*/
void OfflineMsgEngine::deliverOfflineMsgs()
{
    QMutexLocker ml(&mutex);

    if (!Settings::getInstance().getFauxOfflineMessaging()) {
        return;
    }

    if (!f->isOnline()) {
        return;
    }

    if (sentSavedMessages.empty() && unsentSavedMessages.empty()) {
        return;
    }

    QVector<Message> messages = sentSavedMessages.values().toVector() + unsentSavedMessages;
    // order messages by authorship time to resend in same order as they were written
    qSort(messages.begin(), messages.end(), [](const Message& lhs, const Message& rhs){ return lhs.authorshipTime < rhs.authorshipTime; });
    removeAllMessages();

    for (const auto& message : messages) {
        QString messageText = message.chatMessage->toString();
        ReceiptNum receipt;
        bool messageSent{false};
        if (message.chatMessage->isAction()) {
            messageSent = Core::getInstance()->sendAction(f->getId(), messageText, receipt);
        } else {
            messageSent = Core::getInstance()->sendMessage(f->getId(), messageText, receipt);
        }
        if (messageSent) {
            addSentSavedMessage(receipt, message.rowId, message.chatMessage);
        } else {
            qCritical() << "deliverOfflineMsgs failed to send message";
            addSavedMessage(message.rowId, message.chatMessage);
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
    sentSavedMessages.clear();
    unsentSavedMessages.clear();
}

void OfflineMsgEngine::completeMessage(QMap<ReceiptNum, Message>::iterator msgIt)
{
    Profile* const profile = Nexus::getProfile();
    if (profile->isHistoryEnabled()) {
        profile->getHistory()->markAsSent(msgIt->rowId);
    }

    if (QThread::currentThread() == QCoreApplication::instance()->thread()) {
        updateTimestamp(msgIt->chatMessage);
    } else {
        QMetaObject::invokeMethod(this, "updateTimestamp", Qt::QueuedConnection, Q_ARG(ChatMessage::Ptr, msgIt->chatMessage));
    }
    sentSavedMessages.erase(msgIt);
    receivedReceipts.removeOne(msgIt.key());
}

void OfflineMsgEngine::checkForCompleteMessages(ReceiptNum receipt)
{
    auto msgIt = sentSavedMessages.find(receipt);
    const bool receiptReceived = receivedReceipts.contains(receipt);
    if (!receiptReceived || msgIt == sentSavedMessages.end()) {
        return;
    }
    assert(!unsentSavedMessages.contains(*msgIt));
    completeMessage(msgIt);
}

void OfflineMsgEngine::updateTimestamp(ChatMessage::Ptr msg)
{
    msg->markAsSent(QDateTime::currentDateTime());
}
