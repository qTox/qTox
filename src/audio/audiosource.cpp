#include "audiosource.h"

#include "src/audio/audio.h"

/**
 * @brief Emits audio frames captured by an input device or other audio source.
 */

/**
 * @brief Reserves ressources for an audio source
 * @param audio Main audio object, must have longer lifetime than this object.
 */
AudioSource::AudioSource(Audio &audio)
    : audio{&audio}
{
    audio.subscribeInput();
}

AudioSource &AudioSource::operator=(AudioSource && other)
{
    audio = other.audio;
    other.audio = nullptr;
    return *this;
}

AudioSource::~AudioSource()
{
    if(audio != nullptr) {
        audio->unsubscribeInput();
    }
}

AudioSource::operator bool() const
{
    return audio != nullptr;
}


AudioSource::AudioSource(AudioSource &&other)
    : audio{other.audio}
{
    other.audio = nullptr;
}
