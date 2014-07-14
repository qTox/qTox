#ifndef AUDIOOUTPUTPROXY_H
#define AUDIOOUTPUTPROXY_H

#include <QIODevice>
#include "memring.h"

class AudioOutputProxy : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioOutputProxy(QObject *parent = 0);
    virtual ~AudioOutputProxy();

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;
    bool isSequential() const;

private:
    MemRing<int16_t> *ring_buffer;
};

#endif // AUDIOOUTPUTPROXY_H
