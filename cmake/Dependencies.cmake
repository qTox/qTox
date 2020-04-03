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
# :: Dependencies
#
################################################################################

# This should go into subdirectories later.
find_package(PkgConfig        REQUIRED)
find_package(Qt5Concurrent    REQUIRED)
find_package(Qt5Core          REQUIRED)
find_package(Qt5Gui           REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Network       REQUIRED)
find_package(Qt5OpenGL        REQUIRED)
find_package(Qt5Svg           REQUIRED)
find_package(Qt5Test          REQUIRED)
find_package(Qt5Widgets       REQUIRED)
find_package(Qt5Xml           REQUIRED)

function(add_dependency)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} ${ARGN} PARENT_SCOPE)
endfunction()

# Everything links to these Qt libraries.
add_dependency(
  Qt5::Core
  Qt5::Gui
  Qt5::Network
  Qt5::OpenGL
  Qt5::Svg
  Qt5::Widgets
  Qt5::Xml)

include(CMakeParseArguments)
include(Qt5CorePatches)

function(search_dependency pkg)
  set(options OPTIONAL STATIC_PACKAGE)
  set(oneValueArgs PACKAGE LIBRARY FRAMEWORK HEADER)
  set(multiValueArgs)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Try pkg-config first.
  if(NOT ${pkg}_FOUND AND arg_PACKAGE)
    pkg_check_modules(${pkg} ${arg_PACKAGE})
  endif()

  # Then, try OSX frameworks.
  if(NOT ${pkg}_FOUND AND arg_FRAMEWORK)
    find_library(${pkg}_LIBRARIES
            NAMES ${arg_FRAMEWORK}
            PATHS ${CMAKE_OSX_SYSROOT}/System/Library
            PATH_SUFFIXES Frameworks
            NO_DEFAULT_PATH
    )
    if(${pkg}_LIBRARIES)
      set(${pkg}_FOUND TRUE)
    endif()
  endif()

  # Last, search for the library itself globally.
  if(NOT ${pkg}_FOUND AND arg_LIBRARY)
    find_library(${pkg}_LIBRARIES NAMES ${arg_LIBRARY})
    if(arg_HEADER)
        find_path(${pkg}_INCLUDE_DIRS NAMES ${arg_HEADER})
    endif()
    if(${pkg}_LIBRARIES AND (${pkg}_INCLUDE_DIRS OR NOT arg_HEADER))
      set(${pkg}_FOUND TRUE)
    endif()
  endif()

  if(NOT ${pkg}_FOUND)
    if(NOT arg_OPTIONAL)
      message(FATAL_ERROR "${pkg} package, library or framework not found")
    else()
      message(STATUS "${pkg} not found")
    endif()
  else()
    if(arg_STATIC_PACKAGE)
      set(maybe_static _STATIC)
    else()
      set(maybe_static "")
    endif()

    message(STATUS ${pkg} " LIBRARY_DIRS: " "${${pkg}${maybe_static}_LIBRARY_DIRS}" )
    message(STATUS ${pkg} " INCLUDE_DIRS: " "${${pkg}${maybe_static}_INCLUDE_DIRS}" )
    message(STATUS ${pkg} " CFLAGS_OTHER: " "${${pkg}${maybe_static}_CFLAGS_OTHER}" )
    message(STATUS ${pkg} " LIBRARIES:    " "${${pkg}${maybe_static}_LIBRARIES}" )

    link_directories(${${pkg}${maybe_static}_LIBRARY_DIRS})
    include_directories(${${pkg}${maybe_static}_INCLUDE_DIRS})

    foreach(flag ${${pkg}${maybe_static}_CFLAGS_OTHER})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
    endforeach()

    set(ALL_LIBRARIES ${ALL_LIBRARIES} ${${pkg}${maybe_static}_LIBRARIES} PARENT_SCOPE)
    message(STATUS "${pkg} found")
  endif()

  set(${pkg}_FOUND ${${pkg}_FOUND} PARENT_SCOPE)
endfunction()

search_dependency(LIBAVCODEC          PACKAGE libavcodec)
search_dependency(LIBAVDEVICE         PACKAGE libavdevice)
search_dependency(LIBAVFORMAT         PACKAGE libavformat)
search_dependency(LIBAVUTIL           PACKAGE libavutil)
search_dependency(LIBEXIF             PACKAGE libexif)
search_dependency(LIBQRENCODE         PACKAGE libqrencode)
search_dependency(LIBSODIUM           PACKAGE libsodium)
search_dependency(LIBSWSCALE          PACKAGE libswscale)
search_dependency(SQLCIPHER           PACKAGE sqlcipher)
search_dependency(VPX                 PACKAGE vpx)

if(${SPELL_CHECK})
    find_package(KF5Sonnet)
    if(KF5Sonnet_FOUND)
      add_definitions(-DSPELL_CHECKING)
      add_dependency(KF5::SonnetUi)
    else()
      message(WARNING "Sonnet not found. Spell checking will be disabled.")
    endif()
endif()

# Try to find cmake toxcore libraries
if(WIN32)
  search_dependency(TOXCORE             PACKAGE toxcore          OPTIONAL STATIC_PACKAGE)
  search_dependency(TOXAV               PACKAGE toxav            OPTIONAL STATIC_PACKAGE)
  search_dependency(TOXENCRYPTSAVE      PACKAGE toxencryptsave   OPTIONAL STATIC_PACKAGE)
else()
  search_dependency(TOXCORE             PACKAGE toxcore          OPTIONAL)
  search_dependency(TOXAV               PACKAGE toxav            OPTIONAL)
  search_dependency(TOXENCRYPTSAVE      PACKAGE toxencryptsave   OPTIONAL)
endif()

# If not found, use automake toxcore libraries
# We only check for TOXCORE, because the other two are gone in 0.2.0.
if (NOT TOXCORE_FOUND)
  search_dependency(TOXCORE         PACKAGE libtoxcore)
  search_dependency(TOXAV           PACKAGE libtoxav)
endif()

search_dependency(OPENAL              PACKAGE openal)

if (PLATFORM_EXTENSIONS AND UNIX AND NOT APPLE)
  # Automatic auto-away support. (X11 also using for capslock detection)
  search_dependency(X11               PACKAGE x11 OPTIONAL)
  search_dependency(XSS               PACKAGE xscrnsaver OPTIONAL)
endif()

if(APPLE)
  search_dependency(AVFOUNDATION      FRAMEWORK AVFoundation)
  search_dependency(COREMEDIA         FRAMEWORK CoreMedia   )
  search_dependency(COREGRAPHICS      FRAMEWORK CoreGraphics)

  search_dependency(FOUNDATION        FRAMEWORK Foundation  OPTIONAL)
  search_dependency(IOKIT             FRAMEWORK IOKit       OPTIONAL)
endif()

if(WIN32)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} strmiids)
  # Qt doesn't provide openssl on windows
  search_dependency(OPENSSL           PACKAGE openssl)
endif()

if (NOT GIT_DESCRIBE)
  execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT GIT_DESCRIBE)
    set(GIT_DESCRIBE "Nightly")
  endif()
endif()

add_definitions(
  -DGIT_DESCRIBE="${GIT_DESCRIBE}"
)

if (NOT GIT_VERSION)
  execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT GIT_VERSION)
    set(GIT_VERSION "build without git")
  endif()
endif()

add_definitions(
  -DGIT_VERSION="${GIT_VERSION}"
)

set(APPLE_EXT False)
if (FOUNDATION_FOUND AND IOKIT_FOUND)
  set(APPLE_EXT True)
endif()

set(X11_EXT False)
if (X11_FOUND AND XSS_FOUND)
  set(X11_EXT True)
endif()

if (PLATFORM_EXTENSIONS)
  if (${APPLE_EXT} OR ${X11_EXT} OR WIN32)
    add_definitions(
      -DQTOX_PLATFORM_EXT
    )
    message(STATUS "Using platform extensions")
  else()
    message(WARNING "Not using platform extensions, dependencies not found")
    set(PLATFORM_EXTENSIONS OFF)
  endif()
endif()

if (${DESKTOP_NOTIFICATIONS})
    # snorenotify does only provide a cmake find module
    find_package(LibsnoreQt5 0.7.0 REQUIRED)
    set(ALL_LIBRARIES ${ALL_LIBRARIES} Snore::Libsnore)
endif()

add_definitions(
  -DLOG_TO_FILE=1
)
