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
    const qint64 total = qMin((qint64)buffer.size(), len);
    memcpy(data, buffer.constData(), total);
    buffer = buffer.mid(total);
    return total;
}

qint64 AudioBuffer::writeData(const char* data, qint64 len)
{
    buffer.append(data, len);
    return len;
}

qint64 AudioBuffer::bytesAvailable() const
{
    return buffer.size() + QIODevice::bytesAvailable();
}

qint64 AudioBuffer::bufferSize() const
{
    return buffer.size();
}

void AudioBuffer::clear()
{
    buffer.clear();
}
