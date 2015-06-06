/*
    Copyright © 2014-2015 by The qTox Project

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
#include "src/friend.h"
#include "src/persistence/historykeeper.h"
#include "src/persistence/settings.h"
#include "src/core/core.h"
#include <QMutexLocker>
#include <QTimer>

const int OfflineMsgEngine::offlineTimeout = 2000;
QMutex OfflineMsgEngine::globalMutex;

OfflineMsgEngine::OfflineMsgEngine(Friend *frnd) :
    mutex(QMutex::Recursive),
    f(frnd)
{
}

OfflineMsgEngine::~OfflineMsgEngine()
{
}

void OfflineMsgEngine::dischargeReceipt(int receipt)
{
    QMutexLocker ml(&mutex);

    auto it = receipts.find(receipt);
    if (it != receipts.end())
    {
        int mID = it.value();
        auto msgIt = undeliveredMsgs.find(mID);
        if (msgIt != undeliveredMsgs.end())
        {
            HistoryKeeper::getInstance()->markAsSent(mID);
            msgIt.value().msg->markAsSent(QDateTime::currentDateTime());
            undeliveredMsgs.erase(msgIt);
        }
        receipts.erase(it);
    }
}

void OfflineMsgEngine::registerReceipt(int receipt, int messageID, ChatMessage::Ptr msg, const QDateTime &timestamp)
{
    QMutexLocker ml(&mutex);

    receipts[receipt] = messageID;
    undeliveredMsgs[messageID] = {msg, timestamp, receipt};
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

    QMap<int, MsgPtr> msgs = undeliveredMsgs;
    removeAllReciepts();

    for (auto iter = msgs.begin(); iter != msgs.end(); iter++)
    {
        if (iter.value().timestamp.msecsTo(QDateTime::currentDateTime()) < offlineTimeout)
        {
            registerReceipt(iter.value().receipt, iter.key(), iter.value().msg, iter.value().timestamp);
            continue;
        }
        QString messageText = iter.value().msg->toString();
        int rec;
        if (iter.value().msg->isAction())
            rec = Core::getInstance()->sendAction(f->getFriendID(), messageText);
        else
            rec = Core::getInstance()->sendMessage(f->getFriendID(), messageText);

        registerReceipt(rec, iter.key(), iter.value().msg);
    }
}

void OfflineMsgEngine::removeAllReciepts()
{
    QMutexLocker ml(&mutex);

    receipts.clear();
    undeliveredMsgs.clear();
}
