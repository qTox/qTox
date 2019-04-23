#ifndef ALSINK_H
#define ALSINK_H

#include <QMutex>
#include <QObject>

#include "src/audio/iaudiosink.h"

class OpenAL;
class QMutex;
class AlSink : public IAudioSink
{
    Q_OBJECT
public:
    AlSink(OpenAL& al, uint sourceId);
    AlSink(const AlSink& src) = delete;
    AlSink& operator=(const AlSink&) = delete;
    AlSink(AlSink&& other) = delete;
    AlSink& operator=(AlSink&& other) = delete;
    ~AlSink();

    void playAudioBuffer(const int16_t* data, int samples, unsigned channels, int sampleRate) const;
    void playMono16Sound(const IAudioSink::Sound& sound);
    void startLoop();
    void stopLoop();

    operator bool() const;

    uint getSourceId() const;
    void kill();

private:
    OpenAL& audio;
    uint sourceId;
    bool killed = false;
    mutable QMutex killLock;
};

#endif // ALSINK_H
