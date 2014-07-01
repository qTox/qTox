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
