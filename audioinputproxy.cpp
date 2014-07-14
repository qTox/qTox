#include "audioinputproxy.h"

#include <QDebug>

#define RING_SIZE 44100*2*2

AudioInputProxy::AudioInputProxy(QObject *parent) :
    QIODevice(parent),
    callback(nullptr),
    ring_buffer(new MemRing<int16_t>(RING_SIZE))
{
    open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    qDebug() << "AudioInputProxy::AudioInputProxy";
}

AudioInputProxy::~AudioInputProxy()
{
    if (ring_buffer) {
        delete ring_buffer;
        ring_buffer = 0;
    }
    close();
    qDebug() << "AudioInputProxy::~AudioInputProxy";
}

qint64 AudioInputProxy::readData(char *data, qint64 len)
{
    qDebug() << "AudioInputProxy::read" << len;
    return ring_buffer->pull((int16_t*)data, len/2)*2;
}

qint64 AudioInputProxy::writeData(const char* data, qint64 len)
{
    qDebug() << "AudioInputProxy::write" << len;
    auto ret = ring_buffer->push((int16_t*)data, len/2)*2;
    if (callback != nullptr) {
        callback();
    }
    return ret;
}

qint64 AudioInputProxy::bytesAvailable() const
{
    return ring_buffer->readSpace() + QIODevice::bytesAvailable();
}

bool AudioInputProxy::isSequential() const
{
    return true;
}
