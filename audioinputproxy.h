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

    inline size_t pull(int16_t *data, size_t len) { return ring_buffer->pull(data, len); }
    inline size_t push(int16_t *data, size_t len) { return ring_buffer->push(data, len); }
    inline size_t readSpace() { return ring_buffer->readSpace(); }
    inline size_t writeSpace() { return ring_buffer->writeSpace(); }

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;
    bool isSequential() const;

private:
    MemRing<int16_t> *ring_buffer;
};

#endif // AUDIOINPUTPROXY_H
