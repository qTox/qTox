################################################################################
#
# :: Dependencies
#
################################################################################

# This should go into subdirectories later.
find_package(PkgConfig        REQUIRED)
find_package(Qt5Core          REQUIRED)
find_package(Qt5Gui           REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Network       REQUIRED)
find_package(Qt5OpenGL        REQUIRED)
find_package(Qt5Sql           REQUIRED)
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
  Qt5::Sql
  Qt5::Svg
  Qt5::Widgets
  Qt5::Xml)

include(CMakeParseArguments)
include(Qt5CorePatches)

function(search_dependency pkg)
  set(options OPTIONAL)
  set(oneValueArgs PACKAGE LIBRARY FRAMEWORK)
  set(multiValueArgs)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Try pkg-config first.
  if(NOT ${pkg}_FOUND AND arg_PACKAGE)
    pkg_search_module(${pkg} ${arg_PACKAGE})
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
    if(${pkg}_LIBRARIES)
      set(${pkg}_FOUND TRUE)
    endif()
  endif()

  if(NOT ${pkg}_FOUND)
    if(NOT arg_OPTIONAL)
      message(FATAL_ERROR "${pkg} package, library or framework not found")
    endif()
  else()
    link_directories(${${pkg}_LIBRARY_DIRS})
    include_directories(${${pkg}_INCLUDE_DIRS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${${pkg}_CFLAGS_OTHER}" PARENT_SCOPE)
    set(ALL_LIBRARIES ${ALL_LIBRARIES} ${${pkg}_LIBRARIES} PARENT_SCOPE)
  endif()

  set(${pkg}_FOUND ${${pkg}_FOUND} PARENT_SCOPE)
endfunction()

search_dependency(APPINDICATOR        PACKAGE appindicator-0.1 OPTIONAL)
search_dependency(GDK_PIXBUF          PACKAGE gdk-pixbuf-2.0   OPTIONAL)
search_dependency(GLIB                PACKAGE glib-2.0         OPTIONAL)
search_dependency(GTK                 PACKAGE gtk+-2.0         OPTIONAL)

search_dependency(LIBAVCODEC          PACKAGE libavcodec)
search_dependency(LIBAVDEVICE         PACKAGE libavdevice)
search_dependency(LIBAVFORMAT         PACKAGE libavformat)
search_dependency(LIBAVUTIL           PACKAGE libavutil)
search_dependency(LIBQRENCODE         PACKAGE libqrencode)
search_dependency(LIBSODIUM           PACKAGE libsodium)
search_dependency(LIBSWSCALE          PACKAGE libswscale)
search_dependency(SQLCIPHER           PACKAGE sqlcipher)
search_dependency(TOXCORE             PACKAGE libtoxcore)
search_dependency(TOXAV               PACKAGE libtoxav)
search_dependency(VPX                 PACKAGE vpx)

search_dependency(OPENAL              PACKAGE openal FRAMEWORK OpenAL)

# Automatic auto-away support. (X11 also using for capslock detection)
search_dependency(X11                 PACKAGE x11 OPTIONAL)
search_dependency(XSS                 LIBRARY Xss OPTIONAL)

if(APPLE)
  search_dependency(AVFOUNDATION      FRAMEWORK AVFoundation)
  search_dependency(COREMEDIA         FRAMEWORK CoreMedia   )
  search_dependency(COREGRAPHICS      FRAMEWORK CoreGraphics)

  search_dependency(FOUNDATION        FRAMEWORK Foundation  OPTIONAL)
  search_dependency(IOKIT             FRAMEWORK IOKit       OPTIONAL)
endif()

if(WIN32)
  search_dependency(STRMIIDS          LIBRARY strmiids)
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

if (NOT TIMESTAMP)
  execute_process(
    COMMAND date +%s
    OUTPUT_VARIABLE TIMESTAMP
  )
endif()

set(APPLE_EXT False)
if (FOUNDATION_FOUND AND IOKIT_FOUND)
  set(APPLE_EXT True)
endif()

set(X11_EXT False)
if (X11_FOUND AND XSS_FOUND)
  set(X11_EXT True)
endif()

if (${APPLE_EXT} OR ${X11_EXT})
  add_definitions(
    -DQTOX_PLATFORM_EXT=1
  )
else()
  add_definitions(
    -DQTOX_PLATFORM_EXT=0
  )
endif()

add_definitions(
  -DTIMESTAMP=${TIMESTAMP}
  -DLOG_TO_FILE=1
)
