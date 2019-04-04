#include "src/audio/backend/alsink.h"
#include "src/audio/backend/openal.h"

#include <QDebug>

/**
 * @brief Can play audio via the speakers or some other audio device. Allocates
 *        and frees the audio ressources internally.
 */

AlSink::~AlSink()
{
    // unsubscribe only if not already killed
    if(killLock->tryLock() && audio != nullptr) {
        audio->destroySink(*this);
    }
}

void AlSink::playAudioBuffer(const int16_t *data, int samples, unsigned channels, int sampleRate) const
{
    if(!killLock->tryLock()) {
        qDebug() << "Sink got killed";
        return;
    }

    if(audio == nullptr) {
        qCritical() << "Trying to play audio on an invalid sink";
    } else {
        audio->playAudioBuffer(sourceId, data, samples, channels, sampleRate);
    }

    killLock->unlock();
}

void AlSink::playMono16Sound(const IAudioSink::Sound &sound)
{
    if(!killLock->tryLock()) {
        qDebug() << "Sink got killed";
        return;
    }

    if(audio == nullptr) {
        qCritical() << "Trying to play sound on an invalid sink";
    } else {
        audio->playMono16Sound(*this, sound);
    }

    killLock->unlock();
}

void AlSink::startLoop()
{
    if(!killLock->tryLock()) {
        qDebug() << "Sink got killed";
        return;
    }

    if(audio == nullptr) {
        qCritical() << "Trying to start loop on an invalid sink";
    } else {
        audio->startLoop(sourceId);
    }

    killLock->unlock();
}

void AlSink::stopLoop()
{
    if(!killLock->tryLock()) {
        qDebug() << "Sink got killed";
        return;
    }

    if(audio == nullptr) {
        qCritical() << "Trying to stop loop on an invalid sink";
    } else {
        audio->stopLoop(sourceId);
    }

    killLock->unlock();
}

uint AlSink::getSourceId() const
{
    if(!killLock->tryLock()) {
        qDebug() << "Sink got killed";
    }

    uint tmp = sourceId;
    killLock->unlock();
    return tmp;
}

void AlSink::kill()
{
    // this lock is only locked once here, afterwards the object is considered dead
    killLock->lock();
    audio = nullptr;
    emit invalidated();
}

AlSink::AlSink(OpenAL& al, uint sourceId)
    : audio{&al}
    , sourceId{sourceId}
    , killLock{new QMutex(QMutex::Recursive)}
{
    assert(killLock != nullptr);
}

AlSink::operator bool() const
{
    // no need to make this thread safe here, because it's not used by the audio thread
    return audio != nullptr;
}
