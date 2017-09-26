################################################################################
#
# :: Process Code
#
################################################################################

# set correct version to update files
set(QTOX_VERSION "1.11.0")
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/tools/update-versions.sh
    ${QTOX_VERSION})

# format the code when clang-format is available, but don't fail if
# it's not available
find_program(CLANG_FORMAT_FOUND ccache)
if(NOT CLANG_FORMAT_FOUND)
  message(STATUS "clang-format not found!")
else()
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/tools/format-code.sh)
endif()
