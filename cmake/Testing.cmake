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

function(auto_test subsystem module extra_libs extra_deps)
  add_executable(test_${module}
    test/${subsystem}/${module}_test.cpp ${extra_deps})
  target_link_libraries(test_${module}
    ${extra_libs}
    ${CHECK_LIBRARIES}
    Qt5::Test)
  add_test(
    NAME test_${module}
    COMMAND ${TEST_CROSSCOMPILING_EMULATOR} test_${module})
endfunction()

auto_test(core core "core_library;${PROJECT_NAME}_static" ${${PROJECT_NAME}_RESOURCES})
auto_test(core contactid "core_library" "")
auto_test(core toxid core_library "")
auto_test(chatlog textformatter "${PROJECT_NAME}_static" "")
auto_test(net bsu "Qt5::Network;core_library;${PROJECT_NAME}_static" ${${PROJECT_NAME}_RESOURCES}) # needs nodes list
auto_test(persistence paths "${PROJECT_NAME}_static" "")
auto_test(persistence dbschema "util_library;core_library;${PROJECT_NAME}_static" "")
auto_test(persistence offlinemsgengine "core_library;${PROJECT_NAME}_static" "")
auto_test(model friendmessagedispatcher "core_library;${PROJECT_NAME}_static" "")
auto_test(model groupmessagedispatcher "core_library;${PROJECT_NAME}_static" "")
auto_test(model sessionchatlog "core_library;${PROJECT_NAME}_static" "")
auto_test(model exiftransform "Qt5::Gui;${PROJECT_NAME}_static" "")
auto_test(model notificationgenerator "Qt5::Gui;core_library;${PROJECT_NAME}_static" "")

# uses internal header file
#auto_test(core toxstring core_libary "")

# possible circular dependency?
#auto_test(model messageprocessor "" "")


if (UNIX)
  auto_test(platform posixsignalnotifier "${PROJECT_NAME}_static" "")
endif()
