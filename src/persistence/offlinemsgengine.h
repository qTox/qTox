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

#ifndef OFFLINEMSGENGINE_H
#define OFFLINEMSGENGINE_H

#include "src/chatlog/chatmessage.h"
#include "src/core/core.h"
#include "src/persistence/db/rawdatabase.h"
#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>

class Friend;

class OfflineMsgEngine : public QObject
{
    Q_OBJECT
public:
    explicit OfflineMsgEngine(Friend*);
    virtual ~OfflineMsgEngine() = default;

    void dischargeReceipt(ReceiptNum receipt);
    void registerReceipt(ReceiptNum receipt, RowId messageID, ChatMessage::Ptr msg);
    void deliverOfflineMsgs();

public slots:
    void removeAllReceipts();
    void updateTimestamp(ReceiptNum receiptId);

private:
    void processReceipt(ReceiptNum receiptId);
    struct Receipt
    {
        bool bRowValid{false};
        RowId rowId{0};
        bool bRecepitReceived{false};
    };

    struct MsgPtr
    {
        ChatMessage::Ptr msg;
        ReceiptNum receipt;
    };
    QMutex mutex;
    Friend* f;
    QHash<ReceiptNum, Receipt> receipts;
    QMap<RowId, MsgPtr> undeliveredMsgs;

    static const int offlineTimeout;
};

#endif // OFFLINEMSGENGINE_H
