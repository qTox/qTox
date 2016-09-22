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

#include "src/core/core.h"
#include "src/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"

#include <QMutexLocker>

QMutex OfflineMsgEngine::globalMutex;

class OfflineMsgEngine::Private
{
public:
    Private(Friend* frnd)
        : f(frnd)
    {
    }

    void registerReceipt(int receipt, int64_t messageID, ChatMessage::Ptr msg,
                         const QDateTime& ts = QDateTime::currentDateTime())
    {
        receipts[receipt] = messageID;
        undeliveredMsgs[messageID] = {msg, ts, receipt};
    }

public:
    Friend* f;
    QHash<int, int64_t> receipts;
    QMap<int64_t, MsgPtr> undeliveredMsgs;
    mutable QMutex mutex;
};

OfflineMsgEngine::OfflineMsgEngine(Friend *frnd)
    : d(new Private(frnd))
{
}

OfflineMsgEngine::~OfflineMsgEngine()
{
    delete d;
}

void OfflineMsgEngine::dischargeReceipt(int receipt)
{
    QMutexLocker ml(&d->mutex);

    Profile* profile = Nexus::getProfile();
    auto it = d->receipts.find(receipt);
    if (it != d->receipts.end())
    {
        int64_t mID = it.value();
        auto msgIt = d->undeliveredMsgs.find(mID);
        if (msgIt != d->undeliveredMsgs.end())
        {
            if (profile->isHistoryEnabled())
                profile->getHistory()->markAsSent(mID);
            msgIt.value().msg->markAsSent(QDateTime::currentDateTime());
            d->undeliveredMsgs.erase(msgIt);
        }
        d->receipts.erase(it);
    }
}

void OfflineMsgEngine::registerReceipt(int receipt, int64_t messageID,
                                       ChatMessage::Ptr msg,
                                       const QDateTime &ts)
{
    QMutexLocker ml(&d->mutex);
    d->registerReceipt(receipt, messageID, msg, ts);
}

void OfflineMsgEngine::deliverOfflineMsgs()
{
    QMutexLocker ml(&d->mutex);

    if (!Settings::getInstance().getFauxOfflineMessaging())
        return;

    if (d->f->getStatus() == Status::Offline)
        return;

    if (d->undeliveredMsgs.size() == 0)
        return;

    QMap<int64_t, MsgPtr> msgs = d->undeliveredMsgs;
    d->receipts.clear();
    d->undeliveredMsgs.clear();

    for (auto iter = msgs.begin(); iter != msgs.end(); ++iter)
    {
        auto val = iter.value();
        auto key = iter.key();

        QString messageText = val.msg->toString();
        int rec;
        if (val.msg->isAction())
            rec = Core::getInstance()->sendAction(d->f->getFriendID(),
                                                  messageText);
        else
            rec = Core::getInstance()->sendMessage(d->f->getFriendID(),
                                                   messageText);

        d->registerReceipt(rec, key, val.msg);
    }
}

void OfflineMsgEngine::removeAllReceipts()
{
    QMutexLocker ml(&d->mutex);
    d->receipts.clear();
}
