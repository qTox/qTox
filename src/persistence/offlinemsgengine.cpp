/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

/**
 * @var static const int OfflineMsgEngine::offlineTimeout
 * @brief timeout after which faux offline messages get to be re-sent.
 * Originally was 2s, but since that was causing lots of duplicated
 * messages on receiving end, make qTox be more lazy about re-sending
 * should be 20s.
 */


const int OfflineMsgEngine::offlineTimeout = 20000;
QMutex OfflineMsgEngine::globalMutex;

OfflineMsgEngine::OfflineMsgEngine(Friend* frnd)
    : mutex(QMutex::Recursive)
    , f(frnd)
{
}

OfflineMsgEngine::~OfflineMsgEngine()
{
}

void OfflineMsgEngine::dischargeReceipt(int receipt)
{
    QMutexLocker ml(&mutex);

    if (receipts.find(receipt) == receipts.end())
    {
        receipts[receipt] = Receipt();
    } else if ( receipts[receipt].bRecepitReceived ){
        qWarning() << "Received duplicate receipt";
    }
    receipts[receipt].bRecepitReceived = true;
    processReceipt_(receipt);
}

void OfflineMsgEngine::registerReceipt(int receipt, int64_t messageID, ChatMessage::Ptr msg,
                                       const QDateTime& timestamp)
{
    QMutexLocker ml(&mutex);

    if (receipts.find(receipt) == receipts.end())
    {
        receipts[receipt] = Receipt();
    } else if ( receipts[receipt].bRowValid && receipt != 0 /* offline receipt */ ){
        qWarning() << "Received duplicate registration of receipt";
    }
    receipts[receipt].rowId = messageID;
    receipts[receipt].bRowValid = true;
    undeliveredMsgs[messageID] = {msg, timestamp, receipt};
    processReceipt_(receipt);
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

    QMap<int64_t, MsgPtr> msgs = undeliveredMsgs;
    removeAllReceipts();
    undeliveredMsgs.clear();

    for (auto iter = msgs.begin(); iter != msgs.end(); ++iter) {
        auto val = iter.value();
        auto key = iter.key();

        if (val.timestamp.msecsTo(QDateTime::currentDateTime()) < offlineTimeout) {
            registerReceipt(val.receipt, key, val.msg, val.timestamp);
            continue;
        }
        QString messageText = val.msg->toString();
        int rec;
        if (val.msg->isAction()) {
            rec = Core::getInstance()->sendAction(f->getId(), messageText);
        } else {
            rec = Core::getInstance()->sendMessage(f->getId(), messageText);
        }

        registerReceipt(rec, key, val.msg);
    }
}

void OfflineMsgEngine::removeAllReceipts()
{
    QMutexLocker ml(&mutex);

    receipts.clear();
}

void OfflineMsgEngine::updateTimestamp(int receiptId)
{
    QMutexLocker ml(&mutex);

    auto receipt = receipts.find(receiptId);
    auto msg = undeliveredMsgs.find(receipt->rowId);
    msg->msg->markAsSent(QDateTime::currentDateTime());
    undeliveredMsgs.remove(receipt->rowId);
    receipts.erase(receipt);
}

void OfflineMsgEngine::processReceipt_(int receiptId)
{
    if (!receipts[receiptId].bRecepitReceived || !receipts[receiptId].bRowValid)
        return;
    Profile* profile = Nexus::getProfile();
    auto receipt = receipts.find(receiptId);
    if (profile->isHistoryEnabled())
        profile->getHistory()->markAsSent(receipt->rowId);

    // could either be called from GUI thread when we receive the receipt, or from db process thread when our message
    // is inserted into history. Can't assume it's safe to update GUI, so need to force GUI thread to update timestamp
    emit setTimestamp(f->getId(), receiptId);
}
