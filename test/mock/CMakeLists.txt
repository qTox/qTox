#   Copyright © 2022 by The qTox Project Contributors
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

add_library(mock_library STATIC
    include/mock/mockcoreidhandler.h
    src/mockcoreidhandler.cpp
    include/mock/mockgroupquery.h
    src/mockgroupquery.cpp
    include/mock/mockcoresettings.h
    src/mockcoresettings.cpp
    include/mock/mockbootstraplistgenerator.h
    src/mockbootstraplistgenerator.cpp)

target_include_directories(mock_library PUBLIC include/)
target_link_libraries(mock_library
  util_library
  Qt5::Core
  Qt5::Network
  Qt5::Gui
  qtox::warnings)
