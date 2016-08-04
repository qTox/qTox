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

#ifndef OFFLINEMSGENGINE_H
#define OFFLINEMSGENGINE_H

#include <QObject>
#include <QSet>
#include <QMutex>
#include <QDateTime>
#include <QMap>
#include "src/chatlog/chatmessage.h"

class Friend;
class QTimer;

class OfflineMsgEngine : public QObject
{
    Q_OBJECT
public:
    explicit OfflineMsgEngine(Friend *);
    virtual ~OfflineMsgEngine();
    static QMutex globalMutex;

    void dischargeReceipt(int receipt);
    void registerReceipt(int receipt, int64_t messageID, ChatMessage::Ptr msg, const QDateTime &timestamp = QDateTime::currentDateTime());

public slots:
    void deliverOfflineMsgs();
    void removeAllReceipts();

private:
    struct MsgPtr {
        ChatMessage::Ptr msg;
        QDateTime timestamp;
        int receipt;
    };

    QMutex mutex;
    Friend* f;
    QHash<int, int64_t> receipts;
    QMap<int64_t, MsgPtr> undeliveredMsgs;

    static const int offlineTimeout;
};

#endif // OFFLINEMSGENGINE_H
