/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include <memory>

#include "audio/audio.h"
#include "audio/iaudiosettings.h"
#include "backend/openal.h"

/**
 * @brief Select the audio backend
 * @param settings Audio settings to use
 * @return Audio backend selection based on settings
 */
std::unique_ptr<IAudioControl> Audio::makeAudio(IAudioSettings& settings)
{
    return std::unique_ptr<IAudioControl>(new OpenAL(settings));
}
