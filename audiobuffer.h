#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <QIODevice>
#include <QByteArray>

class AudioBuffer : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioBuffer();
    ~AudioBuffer();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;
    qint64 bufferSize() const;
    void clear();

private:
    QByteArray buffer;
};

#endif // AUDIOBUFFER_H
