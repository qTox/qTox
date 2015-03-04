/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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


#ifndef IPC_H
#define IPC_H

#include <QSharedMemory>
#include <QTimer>
#include <QObject>
#include <QThread>
#include <QVector>
#include <QMap>
#include <functional>
#include <ctime>

using IPCEventHandler = std::function<bool (const QByteArray&)>;

#define IPC_PROTOCOL_VERSION "2"

class IPC : public QThread
{
    Q_OBJECT
    IPC();
protected:
    static const int EVENT_TIMER_MS = 1000;
    static const int EVENT_GC_TIMEOUT = 5;
    static const int EVENT_QUEUE_SIZE = 32;
    static const int OWNERSHIP_TIMEOUT_S = 5;

public:
    ~IPC();

    static IPC& getInstance();

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
        // When last event was posted
        time_t lastEvent;
        // When processEvents() ran last time
        time_t lastProcessed;
        IPCEvent events[IPC::EVENT_QUEUE_SIZE];
    };

    // dest: Settings::getCurrentProfileId() or 0 (main instance).
    time_t postEvent(const QString& name, const QByteArray &data=QByteArray(), uint32_t dest=0);
    bool isCurrentOwner();
    void registerEventHandler(const QString& name, IPCEventHandler handler);
    bool isEventProcessed(time_t time);
    bool isEventAccepted(time_t time);
    bool waitUntilProcessed(time_t time, int32_t timeout=-1);

protected slots:
    void processEvents();

protected:
    IPCMemory* global();
    bool runEventHandler(IPCEventHandler handler, const QByteArray& arg);
    // Only called when global memory IS LOCKED, returns 0 if no evnts present
    IPCEvent* fetchEvent();

    QTimer timer;
    uint64_t globalId;
    QSharedMemory globalMemory;
    QMap<QString, IPCEventHandler> eventHandlers;
};

#endif // IPC_H
