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

OfflineMsgEngine::OfflineMsgEngine(Friend* frnd)
    : mutex(QMutex::Recursive)
    , f(frnd)
{
}

void OfflineMsgEngine::onReceiptReceived(ReceiptNum receipt)
{
    QMutexLocker ml(&mutex);
    assert(!receivedReceipts.contains(receipt));
    receivedReceipts.append(receipt);
    checkForCompleteMessages(receipt);
}

void OfflineMsgEngine::addSavedMessage(RowId messageID, ChatMessage::Ptr chatMessage)
{
    QMutexLocker ml(&mutex);
    unsentSavedMessages.append(Message{chatMessage, messageID});
}

void OfflineMsgEngine::addSentSavedMessage(ReceiptNum receipt, RowId messageID, ChatMessage::Ptr chatMessage)
{
    QMutexLocker ml(&mutex);
    assert(!sentSavedMessages.contains(receipt));
    sentSavedMessages.insert(receipt, {chatMessage, messageID});
    checkForCompleteMessages(receipt);
}

void OfflineMsgEngine::deliverOfflineMsgs()
{
    QMutexLocker ml(&mutex);

    if (!Settings::getInstance().getFauxOfflineMessaging())
        return;

    if (f->getStatus() == Status::Offline)
        return;

    if (sentSavedMessages.empty() && unsentSavedMessages.empty())
        return;

    QVector<Message> messages = sentSavedMessages.values().toVector() + unsentSavedMessages;
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
