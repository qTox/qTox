#include "alsource.h"

#include "src/audio/backend/openal.h"

/**
 * @brief Emits audio frames captured by an input device or other audio source.
 */

/**
 * @brief Reserves ressources for an audio source
 * @param audio Main audio object, must have longer lifetime than this object.
 */
AlSource::AlSource(OpenAL &al)
    : audio{&al}
    , killLock{new QMutex}
{
    assert(killLock != nullptr);
}

AlSource::~AlSource()
{
    // unsubscribe only if not already killed
    if(killLock->tryLock() && audio != nullptr) {
        audio->destroySource(*this);
    }
}

AlSource::operator bool() const
{
    // no need to make this thread safe here, because it's not used by the audio thread
    return audio != nullptr;
}

void AlSource::kill()
{
    // this lock is only locked once here, afterwards the object is considered dead
    killLock->lock();
    audio = nullptr;
    emit invalidated();
}


