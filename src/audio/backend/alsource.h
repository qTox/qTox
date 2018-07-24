#ifndef ALSOURCE_H
#define ALSOURCE_H

#include "src/audio/iaudiosource.h"

class OpenAL;
class QMutex;
class AlSource : public IAudioSource
{
    Q_OBJECT
public:
    AlSource(OpenAL& al);
    AlSource(AlSource &src) = delete;
    AlSource & operator=(const AlSource&) = delete;
    AlSource(AlSource&& other) = delete;
    AlSource & operator=(AlSource&& other) = delete;
    ~AlSource();

    operator bool() const;

    void kill();

signals:
    void frameAvailable(const int16_t* pcm, size_t sample_count, uint8_t channels,
                        uint32_t sampling_rate);
    void invalidated();

private:
    OpenAL* audio;
    QMutex* killLock;
};

#endif // ALSOURCE_H
