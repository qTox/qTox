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

#include "audio/src/backend/alsource.h"
#include "audio/src/backend/openal.h"

/**
 * @brief Emits audio frames captured by an input device or other audio source.
 */

/**
 * @brief Reserves ressources for an audio source
 * @param audio Main audio object, must have longer lifetime than this object.
 */
AlSource::AlSource(OpenAL& al)
    : audio(al)
    , killLock(QMutex::Recursive)
{}

AlSource::~AlSource()
{
    QMutexLocker{&killLock};

    // unsubscribe only if not already killed
    if (!killed) {
        audio.destroySource(*this);
        killed = true;
    }
}

AlSource::operator bool() const
{
    QMutexLocker{&killLock};
    return !killed;
}

void AlSource::kill()
{
    killLock.lock();
    // this flag is only set once here, afterwards the object is considered dead
    killed = true;
    killLock.unlock();
    emit invalidated();
}
