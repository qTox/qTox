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

void OfflineMsgEngine::dischargeReceipt(ReceiptNum receipt)
{
    QMutexLocker ml(&mutex);

    auto it = receipts.find(receipt);
    if (it == receipts.end()) {
        it = receipts.insert(receipt, Receipt());
    } else if (it->bRecepitReceived) {
        qWarning() << "Received duplicate receipt";
    }
    it->bRecepitReceived = true;
    processReceipt(receipt);
}

void OfflineMsgEngine::registerReceipt(ReceiptNum receipt, RowId messageID, ChatMessage::Ptr msg)
{
    QMutexLocker ml(&mutex);

    auto it = receipts.find(receipt);
    if (it == receipts.end()) {
        it = receipts.insert(receipt, Receipt());
    } else if (it->bRowValid && receipt.get() != 0 /* offline receipt */) {
        qWarning() << "Received duplicate registration of receipt";
    }
    it->rowId = messageID;
    it->bRowValid = true;
    undeliveredMsgs[messageID] = {msg, receipt};
    processReceipt(receipt);
}

void OfflineMsgEngine::deliverOfflineMsgs()
{
    QMutexLocker ml(&mutex);

    if (!Settings::getInstance().getFauxOfflineMessaging())
        return;

    if (f->getStatus() == Status::Offline)
        return;

    if (undeliveredMsgs.size() == 0)
        return;

    QMap<RowId, MsgPtr> msgs = undeliveredMsgs;
    removeAllReceipts();
    undeliveredMsgs.clear();

    for (auto iter = msgs.begin(); iter != msgs.end(); ++iter) {
        auto val = iter.value();
        auto key = iter.key();
        QString messageText = val.msg->toString();
        ReceiptNum rec;
        if (val.msg->isAction()) {
            Core::getInstance()->sendAction(f->getId(), messageText, rec);
        } else {
            Core::getInstance()->sendMessage(f->getId(), messageText, rec);
        }

        registerReceipt(rec, key, val.msg);
    }
}

void OfflineMsgEngine::removeAllReceipts()
{
    QMutexLocker ml(&mutex);

    receipts.clear();
}

void OfflineMsgEngine::updateTimestamp(ReceiptNum receiptId)
{
    QMutexLocker ml(&mutex);

    auto receipt = receipts.find(receiptId);
    const auto msg = undeliveredMsgs.constFind(receipt->rowId);
    if (msg == undeliveredMsgs.end()) {
        // this should never occur as registerReceipt adds the msg before processReceipt calls updateTimestamp
        qCritical() << "Message was not in undeliveredMsgs map when attempting to update its timestamp!";
        return;
    }
    msg->msg->markAsSent(QDateTime::currentDateTime());
    undeliveredMsgs.remove(receipt->rowId);
    receipts.erase(receipt);
}

void OfflineMsgEngine::processReceipt(ReceiptNum receiptId)
{
    const auto receipt = receipts.constFind(receiptId);
    if (receipt == receipts.end()) {
        // this should never occur as callers ensure receipts contains receiptId
        qCritical() << "Receipt was not added to map prior to attempting to process it!";
        return;
    }

    if (!receipt->bRecepitReceived || !receipt->bRowValid)
        return;

    Profile* const profile = Nexus::getProfile();
    if (profile->isHistoryEnabled()) {
        profile->getHistory()->markAsSent(receipt->rowId);
    }

    if (QThread::currentThread() == QCoreApplication::instance()->thread()) {
        updateTimestamp(receiptId);
    } else {
        QMetaObject::invokeMethod(this, "updateTimestamp", Qt::QueuedConnection, Q_ARG(ReceiptNum, receiptId));
    }
}
