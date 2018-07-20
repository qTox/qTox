#ifndef AUDIOSINK_H
#define AUDIOSINK_H

#include <QObject>

#include "src/audio/audio.h"
class AudioSink : public QObject
{
    Q_OBJECT
public:
    AudioSink();
    AudioSink(Audio& audio);
    AudioSink(const AudioSink &src) = delete;
    AudioSink & operator=(const AudioSink&) = delete;
    AudioSink(AudioSink&& other);
    AudioSink & operator=(AudioSink&& other);
    ~AudioSink();

    void playAudioBuffer(const int16_t* data, int samples, unsigned channels,
                         int sampleRate) const;
    void playMono16Sound(const Audio::Sound& sound);
    void startLoop();
    void stopLoop();


    operator bool() const;

private:
    Audio* audio;
    uint sourceId;
};

#endif // AUDIOSINK_H
