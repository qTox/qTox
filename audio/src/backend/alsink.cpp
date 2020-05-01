/*
    Copyright © 2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "audio/src/backend/alsink.h"
#include "audio/src/backend/openal.h"

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
