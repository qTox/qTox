#include "audioinputproxy.h"

#include <QDebug>

#define RING_SIZE 44100*2*2

AudioInputProxy::AudioInputProxy(QObject *parent) :
    QIODevice(parent),
    callback(nullptr),
    ring_buffer(new MemRing<char>(RING_SIZE))
{
    open(QIODevice::ReadWrite);
}

AudioInputProxy::~AudioInputProxy()
{
    if (ring_buffer) {
        delete ring_buffer;
        ring_buffer = 0;
    }
}

qint64 AudioInputProxy::readData(char *data, qint64 len)
{
    qDebug() << "AudioInputProxy::readData" << len;
    return ring_buffer->pull(data, len);
}

qint64 AudioInputProxy::writeData(const char* data, qint64 len)
{
    qDebug() << "AudioInputProxy::writeData" << len;
    ring_buffer->push((char*)data, len);
    if (callback != nullptr) {
        callback();
    }
    return len;
}

qint64 AudioInputProxy::bytesAvailable() const
{
    return ring_buffer->readSpace() + QIODevice::bytesAvailable();
}
