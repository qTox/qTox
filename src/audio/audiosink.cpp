#include "audiosink.h"

#include "src/audio/audio.h"

#include <QDebug>

/**
 * @brief Can play audio via the speakers or some other audio device. Allocates
 *        and frees the audio ressources internally.
 */

/**
 * @brief Reserves ressources for an audio source
 * @param audio Main audio object, must have longer lifetime than this object.
 */
AudioSink::AudioSink(Audio &audio)
    : audio{&audio}
{
    audio.subscribeOutput(sourceId);
}

AudioSink &AudioSink::operator=(AudioSink && other)
{
    audio = other.audio;
    other.audio = nullptr;
    return *this;
}

AudioSink::~AudioSink()
{
    if(audio != nullptr) {
        audio->unsubscribeOutput(sourceId);
    }
}

void AudioSink::playAudioBuffer(const int16_t *data, int samples, unsigned channels, int sampleRate) const
{
    if(audio == nullptr) {
        qCritical() << "Trying to play audio on an invalid sink";
    } else {
        audio->playAudioBuffer(sourceId, data, samples, channels, sampleRate);
    }
}

AudioSink::operator bool() const
{
    return audio != nullptr;
}


AudioSink::AudioSink(AudioSink &&other)
    : audio{other.audio}
{
    other.audio = nullptr;
}
