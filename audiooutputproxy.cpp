#include "audiooutputproxy.h"

#include <QDebug>

#define RING_SIZE 44100*2*2

AudioOutputProxy::AudioOutputProxy(QObject *parent) :
    QIODevice(parent),
    ring_buffer(new MemRing<int16_t>(RING_SIZE))
{
    open(QIODevice::ReadWrite | QIODevice::Unbuffered);
    qDebug() << "AudioOutputProxy::AudioOutputProxy";
}

AudioOutputProxy::~AudioOutputProxy()
{
    if (ring_buffer) {
        delete ring_buffer;
        ring_buffer = 0;
    }
    close();
    qDebug() << "AudioOutputProxy::~AudioOutputProxy";
}

qint64 AudioOutputProxy::readData(char *data, qint64 len)
{
    qDebug() << "AudioOutputProxy::read" << len;
    return ring_buffer->pull((int16_t*)data, len/2)*2;
}

qint64 AudioOutputProxy::writeData(const char* data, qint64 len)
{
//    qDebug() << "AudioOutputProxy::write" << len;
    return ring_buffer->push((int16_t*)data, len/2)*2;
}

qint64 AudioOutputProxy::bytesAvailable() const
{
    return ring_buffer->readSpace() + QIODevice::bytesAvailable();
}

bool AudioOutputProxy::isSequential() const
{
    return true;
}
