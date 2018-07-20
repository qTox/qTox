#ifndef IAUDIOSOURCE_H
#define IAUDIOSOURCE_H

#include <QObject>

class Audio;
class IAudioSource : public QObject
{
    Q_OBJECT
public:
    virtual IAudioSource & operator=(IAudioSource&& other) = 0;
    virtual ~IAudioSource() {}

    virtual operator bool() const = 0;

signals:
    void frameAvailable(const int16_t* pcm, size_t sample_count, uint8_t channels,
                        uint32_t sampling_rate);
};

#endif // IAUDIOSOURCE_H
