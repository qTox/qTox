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

#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <atomic>
#include <cstddef>
#include <stdlib.h>
#include <QIODevice>

template<size_t SIZE>
class AudioBuffer : public QIODevice
{
public:
    enum {capacity = SIZE};
    AudioBuffer(QObject* parent = 0) : QIODevice(parent), tail(0), head(0)
    {
        static_assert(SIZE > 0, "the size of the buffer must be > 0");
        QIODevice::open(QIODevice::ReadWrite);
    }

    qint64 writeData(const char *data, qint64 maxSize)
    {
        if (maxSize < 0) {
            return -1;
        }

        size_t currentTail = tail.load();
        const size_t currentHead = head.load();
        const qint64 available = currentTail >= currentHead ?
            (capacity - (currentTail - currentHead)) - 1 :
            (currentHead - currentTail) - 1;

        maxSize = std::min(maxSize, available);
        qint64 bytesToWrite = maxSize;
        qint64 bytesAlreadyWritten = 0;
        size_t nextTail = currentTail + maxSize;

        if (nextTail < currentTail) {
            // single overflow protection
            return -1;
        } else if (nextTail >= capacity) {
            bytesAlreadyWritten = capacity - currentTail;
            bytesToWrite -= bytesAlreadyWritten;
            memcpy(buffer + currentTail, data, bytesAlreadyWritten * sizeof(char));
            currentTail = 0;
            nextTail = nextTail % capacity;
        }
        memcpy(buffer + currentTail, data + bytesAlreadyWritten, bytesToWrite * sizeof(char));
        tail.store(nextTail);

        emit bytesWritten(maxSize);
        return maxSize;
    }

    qint64 readData(char *data, qint64 maxSize)
    {
        if (maxSize < 0) {
            return -1;
        }

#ifdef _WIN32
        // for more information please read the
        // comment at the end of this function

        // we should ensure here that data doesn't contain any noise
        memset(data, 0, maxSize * sizeof(char));
        // maxSize will be modified, remember it
        const qint64 returnSize = maxSize;
#endif

        const size_t currentTail = tail.load();
        size_t currentHead = head.load();
        const qint64 contains =  currentHead <= currentTail ?
            currentTail - currentHead :
            capacity - (currentHead - currentTail);

        maxSize = std::min(maxSize, contains);
        qint64 bytesToRead = maxSize;
        qint64 bytesAlreadyRead = 0;
        size_t nextHead = currentHead + maxSize;

        if (nextHead < currentHead) {
            // single overflow protection
            return -1;
        } else if (nextHead >= capacity) {
            bytesAlreadyRead = capacity - currentHead;
            bytesToRead -= bytesAlreadyRead;
            memcpy(data, buffer + currentHead, bytesAlreadyRead * sizeof(char));
            currentHead = 0;
            nextHead = nextHead % capacity;
        }
        memcpy(data + bytesAlreadyRead, buffer + currentHead, bytesToRead * sizeof(char));
        head.store(nextHead);

#ifdef _WIN32
        // this hack is necessary for windows since the QAudioOutput stops consuming
        // if the returned value at the first call is equals to 0. Furthermore
        // returning a small value like 1 or 100 at the first call of this function
        // by QAudioOutput will end in laggy sound. In short this hack ensures that:
        //
        // 1. QAudioOutput will not stop consuming bytes
        // 2. The audio output does not lag
        return returnSize;
#else
        // on linux this just works fine
        return maxSize;
#endif
    }

    qint64 bufferSize() const
    {
        const size_t currentTail = tail.load();
        const size_t currentHead = head.load();
        const qint64 contains =  currentHead <= currentTail ?
            currentTail - currentHead :
            capacity - (currentHead - currentTail);
        return contains;
    }

    qint64 bytesAvailable() const
    {
        return bufferSize() + QIODevice::bytesAvailable();
    }

    // Never call this function while there are any producer
    // or consuermer left, otherwise the behaviour is undefined
    void clear()
    {
        tail.store(0);
        head.store(0);
    }

    bool isSequential() const
    {
        return true;
    }

private:
    char buffer[capacity];
    std::atomic<size_t> tail;
    std::atomic<size_t> head;
};

#endif // AUDIOBUFFER_H
