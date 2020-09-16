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

set(SOURCE_FILES
    "include/core/contactid.h"
    "include/core/core.h"
    "include/core/coreav.h"
    "include/core/corefile.h"
    "include/core/dhtserver.h"
    "include/core/groupid.h"
    "include/core/groupinvite.h"
    "include/core/ibootstraplistgenerator.h"
    "include/core/icorefriendmessagesender.h"
    "include/core/icoregroupmessagesender.h"
    "include/core/icoregroupquery.h"
    "include/core/icoreidhandler.h"
    "include/core/icoresettings.h"
    "include/core/itoxstatus.h"
    "include/core/igroupsettings.h"
    "include/core/receiptnum.h"
    "include/core/toxcall.h"
    "include/core/toxencrypt.h"
    "include/core/toxfile.h"
    "include/core/toxfilepause.h"
    "include/core/toxid.h"
    "include/core/toxlogger.h"
    "include/core/toxoptions.h"
    "include/core/toxpk.h"
    "src/contactid.cpp"
    "src/coreav.cpp"
    "src/core.cpp"
    "src/corefile.cpp"
    "src/dhtserver.cpp"
    "src/groupid.cpp"
    "src/groupinvite.cpp"
    "src/toxcall.cpp"
    "src/toxencrypt.cpp"
    "src/toxfile.cpp"
    "src/toxid.cpp"
    "src/toxlogger.cpp"
    "src/toxoptions.cpp"
    "src/toxpk.cpp"
    "src/toxstring.cpp"
    "src/toxstring.h")

add_library(core_library STATIC ${SOURCE_FILES})

# We need this directory, and users of our library will need it too
target_include_directories(core_library PUBLIC include/)

# The variables are set by Dependencies.cmake
target_link_libraries(core_library ${TOXCORE_LIBRARIES})
include_directories(core_library PRIVATE ${TOXCORE_INCLUDE_DIRS})
target_link_libraries(core_library ${TOXAV_LIBRARIES})
include_directories(core_library PRIVATE ${TOXAV_INCLUDE_DIRS})
# in case toxencryptsave is a separate library
if (TOXENCRYPTSAVE_FOUND)
    target_link_libraries(core_library ${TOXENCRYPTSAVE_LIBRARIES})
    include_directories(core_library PRIVATE ${TOXENCRYPTSAVE_INCLUDE_DIRS})
endif()

target_link_libraries(core_library util_library)
target_link_libraries(core_library audio_library)
target_link_libraries(core_library Qt5::Core)
target_link_libraries(core_library Qt5::Network)
target_link_libraries(core_library Qt5::Gui)
