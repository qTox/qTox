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

#pragma once

#include "src/chatlog/chatmessage.h"
#include "src/core/core.h"
#include "src/model/message.h"
#include "src/persistence/db/rawdatabase.h"
#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <chrono>

class Friend;
class ICoreFriendMessageSender;

class OfflineMsgEngine : public QObject
{
    Q_OBJECT
public:
    explicit OfflineMsgEngine(Friend* f, ICoreFriendMessageSender* messageSender);

    using CompletionFn = std::function<void()>;
    void addUnsentMessage(Message const& message, CompletionFn completionCallback);
    void addSentMessage(ReceiptNum receipt, Message const& message, CompletionFn completionCallback);
    void deliverOfflineMsgs();

public slots:
    void removeAllMessages();
    void onReceiptReceived(ReceiptNum receipt);

private:
    struct OfflineMessage
    {
        Message message;
        std::chrono::time_point<std::chrono::steady_clock> authorshipTime;
        CompletionFn completionFn;
    };

private slots:
    void completeMessage(QMap<ReceiptNum, OfflineMessage>::iterator msgIt);

private:
    void checkForCompleteMessages(ReceiptNum receipt);

    QMutex mutex;
    const Friend* f;
    ICoreFriendMessageSender* messageSender;
    QVector<ReceiptNum> receivedReceipts;
    QMap<ReceiptNum, OfflineMessage> sentMessages;
    QVector<OfflineMessage> unsentMessages;
};
