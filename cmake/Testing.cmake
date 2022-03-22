#   Copyright © 2019 by The qTox Project Contributors
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

################################################################################
#
# :: Testing
#
################################################################################

enable_testing()

function(auto_test subsystem module extra_res extra_libs)
  add_executable(test_${module}
    test/${subsystem}/${module}_test.cpp ${extra_res})
  target_link_libraries(test_${module}
    ${PROJECT_NAME}_static
    ${CHECK_LIBRARIES}
    Qt5::Test
    ${extra_libs})
  add_test(
    NAME test_${module}
    COMMAND ${TEST_CROSSCOMPILING_EMULATOR} test_${module})
endfunction()

add_subdirectory(test/mock)
add_subdirectory(test/dbutility)

auto_test(core core "${${PROJECT_NAME}_RESOURCES}" "mock_library")
auto_test(core chatid "" "")
auto_test(core toxid "" "")
auto_test(core toxstring "" "")
auto_test(core fileprogress "" "")
auto_test(chatlog textformatter "" "")
auto_test(net bsu "${${PROJECT_NAME}_RESOURCES}" "") # needs nodes list
auto_test(chatlog chatlinestorage "" "")
auto_test(persistence paths "" "")
auto_test(persistence dbschema "" "dbutility_library")
auto_test(persistence/dbupgrade dbTo11 "" "dbutility_library")
auto_test(persistence offlinemsgengine "" "")
if(NOT "${SMILEYS}" STREQUAL "DISABLED")
  auto_test(persistence smileypack "${SMILEY_RESOURCES}" "") # needs emojione
endif()
auto_test(model friendlistmanager "" "")
auto_test(model friendmessagedispatcher "" "")
auto_test(model groupmessagedispatcher "" "mock_library")
auto_test(model messageprocessor "" "")
auto_test(model sessionchatlog "" "")
auto_test(model exiftransform "" "")
auto_test(model notificationgenerator "" "mock_library")
auto_test(widget filesform "" "")

if (UNIX)
  auto_test(platform posixsignalnotifier "" "")
endif()
