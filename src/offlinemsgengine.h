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

#ifndef OFFLINEMSGENGINE_H
#define OFFLINEMSGENGINE_H

#include <QObject>
#include <QSet>
#include <QMutex>
#include <QDateTime>
#include "src/widget/tool/chatactions/messageaction.h"

struct Friend;
class QTimer;

class OfflineMsgEngine : public QObject
{
    Q_OBJECT
public:
    OfflineMsgEngine(Friend *);
    virtual ~OfflineMsgEngine();
    static QMutex globalMutex;

    void dischargeReceipt(int receipt);
    void registerReceipt(int receipt, int messageID, MessageActionPtr msg, const QDateTime &timestamp = QDateTime::currentDateTime());

public slots:
    void deliverOfflineMsgs();
    void removeAllReciepts();

private:
    struct MsgPtr {
        MessageActionPtr msg;
        QDateTime timestamp;
        int receipt;
    };

    QMutex mutex;
    Friend* f;
    QHash<int, int> receipts;
    QMap<int, MsgPtr> undeliveredMsgs;

    static const int offlineTimeout;
};

#endif // OFFLINEMSGENGINE_H
