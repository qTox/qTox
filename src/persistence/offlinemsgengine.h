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
#include <chrono>

class Friend;

class OfflineMsgEngine : public QObject
{
    Q_OBJECT
public:
    explicit OfflineMsgEngine(Friend*);
    void addSavedMessage(RowId messageID, ChatMessage::Ptr msg);
    void addSentSavedMessage(ReceiptNum receipt, RowId messageID, ChatMessage::Ptr msg);
    void deliverOfflineMsgs();

public slots:
    void removeAllMessages();
    void onReceiptReceived(ReceiptNum receipt);

private:
    struct Message
    {
        bool operator==(const Message& rhs) const { return rhs.rowId == rowId; }
        ChatMessage::Ptr chatMessage;
        RowId rowId;
        std::chrono::time_point<std::chrono::steady_clock> authorshipTime;
    };

private slots:
    void completeMessage(QMap<ReceiptNum, Message>::iterator msgIt);

private:
    void updateTimestamp(ChatMessage::Ptr msg);
    void checkForCompleteMessages(ReceiptNum receipt);

    QMutex mutex;
    const Friend* f;
    QVector<ReceiptNum> receivedReceipts;
    QMap<ReceiptNum, Message> sentSavedMessages;
    QVector<Message> unsentSavedMessages;
};

#endif // OFFLINEMSGENGINE_H
