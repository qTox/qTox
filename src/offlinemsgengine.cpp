/*
    Copyright (C) 2015 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "offlinemsgengine.h"
#include "src/friend.h"
#include "src/historykeeper.h"
#include "src/misc/settings.h"
#include "src/core.h"
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
            msgIt.value().msg->markAsSent();
            msgIt.value().msg->featureUpdate();
            undeliveredMsgs.erase(msgIt);
        }
        receipts.erase(it);
    }
}

void OfflineMsgEngine::registerReceipt(int receipt, int messageID, MessageActionPtr msg, const QDateTime &timestamp)
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
        QString messageText = iter.value().msg->getRawMessage();
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
