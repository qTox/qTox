#ifndef AUDIOINPUTPROXY_H
#define AUDIOINPUTPROXY_H

#include <QIODevice>
#include <functional>
#include "memring.h"

class AudioInputProxy : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioInputProxy(QObject *parent = 0);
    virtual ~AudioInputProxy();

    std::function< void() > callback;
    qint64 bytesAvailable() const;

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    bool isSequential() const;

private:
    MemRing<int16_t> *ring_buffer;
};

#endif // AUDIOINPUTPROXY_H
