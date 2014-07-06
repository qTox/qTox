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

#include "audiobuffer.h"

AudioBuffer::AudioBuffer() :
    QIODevice(0)
{
    open(QIODevice::ReadWrite);
}

AudioBuffer::~AudioBuffer()
{
    close();
}

qint64 AudioBuffer::readData(char *data, qint64 len)
{
    qint64 total;
    bufferMutex.lock();
    try {
        total = qMin((qint64)buffer.size(), len);
        memcpy(data, buffer.constData(), total);
        buffer = buffer.mid(total);
    }
    catch (...)
    {
        bufferMutex.unlock();
        return 0;
    }
    bufferMutex.unlock();
    return total;
}

qint64 AudioBuffer::writeData(const char* data, qint64 len)
{
    bufferMutex.lock();
    try {
        buffer.append(data, len);
    }
    catch (...)
    {
        bufferMutex.unlock();
        return 0;
    }
    bufferMutex.unlock();
    return len;
}

qint64 AudioBuffer::bytesAvailable() const
{
    bufferMutex.lock();
    long long size = buffer.size() + QIODevice::bytesAvailable();
    bufferMutex.unlock();
    return size;
}

qint64 AudioBuffer::bufferSize() const
{
    bufferMutex.lock();
    long long size = buffer.size();
    bufferMutex.unlock();
    return size;
}

void AudioBuffer::clear()
{
    bufferMutex.lock();
    buffer.clear();
    bufferMutex.unlock();
}
