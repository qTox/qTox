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

#include <QMap>
#include <QObject>
#include <QSharedMemory>
#include <QTimer>
#include <ctime>
#include <functional>

using IPCEventHandler = std::function<bool(const QByteArray&, void*)>;

#define IPC_PROTOCOL_VERSION "2"

class IPC : public QObject
{
    Q_OBJECT

protected:
    static const int EVENT_TIMER_MS = 1000;
    static const int EVENT_GC_TIMEOUT = 5;
    static const int EVENT_QUEUE_SIZE = 32;
    static const int OWNERSHIP_TIMEOUT_S = 5;

public:
    IPC(uint32_t profileId_);
    ~IPC();

    struct IPCEvent
    {
        uint32_t dest;
        int32_t sender;
        char name[16];
        char data[128];
        time_t posted;
        time_t processed;
        uint32_t flags;
        bool accepted;
        bool global;
    };

    struct IPCMemory
    {
        uint64_t globalId;
        time_t lastEvent;
        time_t lastProcessed;
        IPCEvent events[IPC::EVENT_QUEUE_SIZE];
    };

    time_t postEvent(const QString& name, const QByteArray& data = QByteArray(), uint32_t dest = 0);
    bool isCurrentOwner();
    void registerEventHandler(const QString& name, IPCEventHandler handler, void* userData);
    bool isEventAccepted(time_t time);
    bool waitUntilAccepted(time_t time, int32_t timeout = -1);
    bool isAttached() const;

public slots:
    void setProfileId(uint32_t profileId_);

private:
    IPCMemory* global();
    bool runEventHandler(IPCEventHandler handler, const QByteArray& arg, void* userData);
    IPCEvent* fetchEvent();
    void processEvents();
    bool isCurrentOwnerNoLock();

private:
    struct Callback
    {
        IPCEventHandler handler;
        void* userData;
    };
    QTimer timer;
    uint64_t globalId;
    uint32_t profileId;
    QSharedMemory globalMemory;
    QMap<QString, Callback> eventHandlers;
};
