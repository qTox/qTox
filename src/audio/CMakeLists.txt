#   Copyright © 2020 by The qTox Project Contributors
#
#   This file is part of qTox, a Qt-based graphical interface for Tox.
#   qTox is libre software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   qTox is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with qTox.  If not, see <http://www.gnu.org/licenses/>

set(FILE_LIST
    "audio.h"
    "audio.cpp"
    "backend/alsink.cpp"
    "backend/alsink.h"
    "backend/alsource.cpp"
    "backend/alsource.h"
    "backend/openal.cpp"
    "backend/openal.h"
    "iaudiocontrol.h"
    "iaudiosettings.h"
    "iaudiosink.h"
    "iaudiosource.h")
    

add_library(audio_library STATIC ${FILE_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(audio_library PUBLIC ./)

target_link_libraries(audio_library ${util_libary})
target_link_libraries(audio_library Qt5::Core)
target_link_libraries(audio_library ${OPENAL_LIBRARIES})

