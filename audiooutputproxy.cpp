#include "audiooutputproxy.h"

#include <QDebug>

#define RING_SIZE 44100*2*2

AudioOutputProxy::AudioOutputProxy(QObject *parent) :
    QIODevice(parent),
    ring_buffer(new MemRing<char>(RING_SIZE))
{
    open(QIODevice::ReadWrite);
}

AudioOutputProxy::~AudioOutputProxy()
{
    if (ring_buffer) {
        delete ring_buffer;
        ring_buffer = 0;
    }

    close();
}

qint64 AudioOutputProxy::readData(char *data, qint64 len)
{
    qDebug() << "AudioOutputProxy::readData" << len;

    size_t ret = ring_buffer->pull(data, len);
    if (ret < (size_t)len) {
        memset(data + ret, 0, len-ret);
    }

    return len; // device will be closed if we will return != len
}

qint64 AudioOutputProxy::writeData(const char* data, qint64 len)
{
    qDebug() << "AudioOutputProxy::writeData" << len;
    if (len > RING_SIZE/8) {
        len = RING_SIZE/8;
    }

    return ring_buffer->push((char*)data, len);
}

qint64 AudioOutputProxy::bytesAvailable() const
{
    return ring_buffer->readSpace() + QIODevice::bytesAvailable();
}
