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


#include "src/ipc.h"
#include <QDebug>
#include <QCoreApplication>

IPC::IPC() :
    globalMemory{"qtox"}
{
    qRegisterMetaType<IPCEventHandler>("IPCEventHandler");

    ownerTimer.setInterval(EVENT_TIMER_MS);
    ownerTimer.setSingleShot(true);
    connect(&ownerTimer, &QTimer::timeout, this, &IPC::processEvents);

    // The first started instance gets to manage the shared memory by taking ownership
    // Every time it processes events it updates the global shared timestamp
    // If the timestamp isn't updated, that's a timeout and someone else can take ownership
    // This is a safety measure, in case one of the clients crashes
    // If the owner exits normally, it can set the timestamp to 0 first to immediately give ownership
    // We keep one shared page, starting with the 64bit ID of the current owner and last timestamp
    // then various events to be processed by the owner with a 16bit size then data each
    // Each event is in its own chunk of data, the last chunk is followed by a chunk of size 0

    qsrand(time(0));
    globalId = ((uint64_t)qrand()) * ((uint64_t)qrand()) * ((uint64_t)qrand());
    qDebug() << "IPC: Our global ID is "<<globalId;
    if (globalMemory.create(MEMORY_SIZE))
    {
        qDebug() << "IPC: Creating the global shared memory and taking ownership";
        if (globalMemory.lock())
        {
            *(uint64_t*)globalMemory.data() = globalId;
            updateGlobalTimestamp();
            globalMemory.unlock();
        }
        else
        {
            qWarning() << "IPC: Couldn't lock to take ownership";
        }
    }
    else if (globalMemory.attach())
    {
        qDebug() << "IPC: Attaching to the global shared memory";
    }
    else
    {
        qDebug() << "IPC: Failed to attach to the global shared memory, giving up";
        return; // We won't be able to do any IPC without being attached, let's get outta here
    }

    ownerTimer.start();
}

IPC::~IPC()
{
    if (globalMemory.lock())
    {
        *(time_t*)((char*)globalMemory.data()+sizeof(globalId)) = 0;
        globalMemory.unlock();
    }
}

bool IPC::isCurrentOwner()
{
    if (globalMemory.lock())
    {
        bool isOwner = ((*(uint64_t*)globalMemory.data()) == globalId);
        globalMemory.unlock();
        return isOwner;
    }
    else
    {
        qWarning() << "IPC:isCurrentOwner failed to lock, returning false";
        return false;
    }
}

void IPC::registerEventHandler(IPCEventHandler handler)
{
    eventHandlers += handler;
}

void IPC::processEvents()
{
    if (globalMemory.lock())
    {
        lastSeenTimestamp = getGlobalTimestamp();

        // Only the owner processes events. But if the previous owner's dead, we can take ownership now
        if (*(uint64_t*)globalMemory.data() != globalId)
        {
            if (difftime(time(0), getGlobalTimestamp()) >= OWNERSHIP_TIMEOUT_S)
            {
                qDebug() << "IPC: Previous owner timed out, taking ownership";
                *(uint64_t*)globalMemory.data() = globalId;
            }
            else
            {
                goto unlockAndRestartTimer;
            }
        }

        // We're the owner, let's process those events
        forever {
            QByteArray eventData = fetchEvent();
            if (eventData.isEmpty())
                break;

            qDebug() << "IPC: Processing event: "<<eventData;
            for (const IPCEventHandler& handler : eventHandlers)
               runEventHandler(handler, eventData);
        }

        updateGlobalTimestamp();
        goto unlockAndRestartTimer;
    }
    else
    {
        //qWarning() << "IPC:processEvents failed to lock";
        goto restartTimer;
    }

    // Centralized cleanup. Always restart the timer, unlock only if we locked successfully.
unlockAndRestartTimer:
    globalMemory.unlock();
restartTimer:
    ownerTimer.start();
    return;
}

time_t IPC::postEvent(const QByteArray& data)
{
    int dataSize = data.size();
    if (dataSize >= 65535)
    {
        qWarning() << "IPC: sendEvent: Too much data for a single chunk, giving up";
        return 0;
    }

    if (globalMemory.lock())
    {
        // Check that we have enough room for that new chunk
        char* nextChunk = getFirstFreeChunk();
        if (nextChunk == nullptr
            || nextChunk + 2 + dataSize + 2 - (char*)globalMemory.data() >= MEMORY_SIZE)
        {
            qWarning() << "IPC: sendEvent: Not enough memory left, giving up";
            return 0;
        }

        // Commit the new chunk to shared memory
        *(uint16_t*)nextChunk = dataSize;
        memcpy(nextChunk+2, data.data(), dataSize);
        *(uint16_t*)(nextChunk+2+dataSize) = 0;

        globalMemory.unlock();
        qDebug() << "IPC: Posted event: "<<data;
        return time(0);
    }
    else
    {
        qWarning() << "IPC: sendEvent failed to lock, giving up";
        return 0;
    }
}

char* IPC::getFirstFreeChunk()
{
    char* ptr = (char*)globalMemory.data() + MEMORY_HEADER_SIZE;

    forever
    {
        uint16_t chunkSize = *(uint16_t*)ptr;

        if (!chunkSize)
            return ptr;

        if ((ptr + chunkSize + 2) - (char*)globalMemory.data() >= MEMORY_SIZE)
            return nullptr;

        ptr += chunkSize;
    }
    return nullptr; // Never reached
}

char* IPC::getLastUsedChunk()
{
    char* ptr = (char*)globalMemory.data() + MEMORY_HEADER_SIZE;
    char* lastPtr = nullptr;

    forever
    {
        uint16_t chunkSize = *(uint16_t*)ptr;

        if (!chunkSize)
            return lastPtr;

        if ((ptr + chunkSize + 2) - (char*)globalMemory.data() > MEMORY_SIZE)
            return lastPtr;

        lastPtr = ptr;
        ptr += chunkSize;
    }
    return nullptr; // Never reached
}

QByteArray IPC::fetchEvent()
{
    QByteArray eventData;

    // Get a pointer to the last chunk
    char* nextChunk = getLastUsedChunk();
    if (nextChunk == nullptr)
        return eventData;

    // Read that chunk and remove it from memory
    uint16_t dataSize = *(uint16_t*)nextChunk;
    *(uint16_t*)nextChunk = 0;
    eventData.resize(dataSize);
    memcpy(eventData.data(), nextChunk+2, dataSize);

    return eventData;
}

void IPC::updateGlobalTimestamp()
{
    *(time_t*)((char*)globalMemory.data()+sizeof(globalId)) = time(0);
}

time_t IPC::getGlobalTimestamp()
{
    return *(time_t*)((char*)globalMemory.data()+sizeof(globalId));
}

bool IPC::isEventProcessed(time_t postTime)
{
    return (difftime(lastSeenTimestamp, postTime) > 0);
}

void IPC::waitUntilProcessed(time_t postTime)
{
    while (difftime(lastSeenTimestamp, postTime) <= 0)
        qApp->processEvents();
}

void IPC::runEventHandler(IPCEventHandler handler, const QByteArray& arg)
{
    if (QThread::currentThread() != qApp->thread())
    {
        QMetaObject::invokeMethod(this, "runEventHandler", Qt::BlockingQueuedConnection,
                                  Q_ARG(IPCEventHandler, handler), Q_ARG(const QByteArray&, arg));
        return;
    }
    else
    {
        handler(arg);
        return;
    }
}
