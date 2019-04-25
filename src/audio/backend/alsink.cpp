#include "src/audio/backend/alsink.h"
#include "src/audio/backend/openal.h"

#include <QDebug>
#include <QMutexLocker>

/**
 * @brief Can play audio via the speakers or some other audio device. Allocates
 *        and frees the audio ressources internally.
 */

AlSink::~AlSink()
{
    QMutexLocker{&killLock};

    // unsubscribe only if not already killed
    if (!killed) {
        audio.destroySink(*this);
        killed = true;
    }
}

void AlSink::playAudioBuffer(const int16_t* data, int samples, unsigned channels, int sampleRate) const
{
    QMutexLocker{&killLock};

    if (killed) {
        qCritical() << "Trying to play audio on an invalid sink";
    } else {
        audio.playAudioBuffer(sourceId, data, samples, channels, sampleRate);
    }
}

void AlSink::playMono16Sound(const IAudioSink::Sound& sound)
{
    QMutexLocker{&killLock};

    if (killed) {
        qCritical() << "Trying to play sound on an invalid sink";
    } else {
        audio.playMono16Sound(*this, sound);
    }
}

void AlSink::startLoop()
{
    QMutexLocker{&killLock};

    if (killed) {
        qCritical() << "Trying to start loop on an invalid sink";
    } else {
        audio.startLoop(sourceId);
    }
}

void AlSink::stopLoop()
{
    QMutexLocker{&killLock};

    if (killed) {
        qCritical() << "Trying to stop loop on an invalid sink";
    } else {
        audio.stopLoop(sourceId);
    }
}

uint AlSink::getSourceId() const
{
    uint tmp = sourceId;
    return tmp;
}

void AlSink::kill()
{
    killLock.lock();
    // this flag is only set once here, afterwards the object is considered dead
    killed = true;
    killLock.unlock();
    emit invalidated();
}

AlSink::AlSink(OpenAL& al, uint sourceId)
    : audio(al)
    , sourceId{sourceId}
    , killLock(QMutex::Recursive)
{}

AlSink::operator bool() const
{
    QMutexLocker{&killLock};

    return !killed;
}
