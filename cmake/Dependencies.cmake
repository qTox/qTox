################################################################################
#
# :: Dependencies
#
################################################################################

# This should go into subdirectories later.
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
  set(oneValueArgs PACKAGE FRAMEWORK)
  set(multiValueArgs LIBRARY HEADER HEADER_SUFFIXES)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Try pkg-config first.
  if(UNIX AND NOT ${pkg}_FOUND AND arg_LIBRARY)
    find_package(PkgConfig)
    foreach(lib ${arg_LIBRARY})
      list(APPEND ${pkg}_CHECK_LIBRARIES lib${lib} ${lib})
    endforeach()
    foreach(pkg_lib ${${pkg}_CHECK_LIBRARIES})
      pkg_check_modules(PC_${pkg} QUIET ${pkg_lib})
      if (PC_${pkg}_FOUND)
        break()
      endif()
    endforeach()
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
    find_library(${pkg}_LIBRARIES NAMES ${arg_LIBRARY} HINTS ${PC_${pkg}_LIBDIR} ${PC_${pkg}_LIBRARY_DIRS})
    if(arg_HEADER)
        find_path(${pkg}_INCLUDE_DIRS NAMES ${arg_HEADER} HINTS ${PC_${pkg}_INCLUDEDIR} ${PC_${pkg}_INCLUDE_DIRS} PATH_SUFFIXES ${arg_HEADER_SUFFIXES})
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

search_dependency(LIBAVCODEC          LIBRARY avcodec HEADER libavcodec/avcodec.h)
search_dependency(LIBAVDEVICE         LIBRARY avdevice HEADER libavdevice/avdevice.h)
search_dependency(LIBAVFORMAT         LIBRARY avformat HEADER libavformat/avformat.h)
search_dependency(LIBAVUTIL           LIBRARY avutil HEADER libavutil/avutil.h)
# Haven't figured out how to build libexif for MSVC yet
if (NOT MSVC)
  search_dependency(LIBEXIF             LIBRARY exif HEADER libexif/exif-loader.h)
  add_definitions("-DUSE_EXIF=1")
endif()
search_dependency(LIBQRENCODE         LIBRARY qrencode HEADER qrencode.h)
search_dependency(LIBSWSCALE          LIBRARY swscale)
search_dependency(SQLCIPHER           LIBRARY sqlcipher libsqlcipher HEADER sqlite3.h HEADER_SUFFIXES sqlcipher)
search_dependency(VPX                 LIBRARY vpx vpxmd HEADER vpx/vpx_codec.h)

# We need to try both static and shared variants of sodium
set(sodium_USE_STATIC_LIBS ON)
find_package(sodium)
if (NOT sodium_FOUND)
  set(sodium_USE_STATIC_LIBS OFF)
  find_package(sodium REQUIRED)
endif()

set(ALL_LIBRARIES ${ALL_LIBRARIES} sodium)

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
  search_dependency(TOXCORE             LIBRARY toxcore          HEADER tox/tox.h OPTIONAL)
  search_dependency(TOXAV               LIBRARY toxav            OPTIONAL)
  search_dependency(TOXENCRYPTSAVE      LIBRARY toxencryptsave   OPTIONAL)
else()
  search_dependency(TOXCORE             LIBRARY toxcore          HEADER tox/tox.h OPTIONAL)
  search_dependency(TOXAV               LIBRARY toxav            OPTIONAL)
  search_dependency(TOXENCRYPTSAVE      LIBRARY toxencryptsave   OPTIONAL)
endif()

# If not found, use automake toxcore libraries
# We only check for TOXCORE, because the other two are gone in 0.2.0.
if (NOT TOXCORE_FOUND)
  search_dependency(TOXCORE         LIBRARY libtoxcore)
  search_dependency(TOXAV           LIBRARY libtoxav)
endif()

search_dependency(OPENAL              LIBRARY openal OpenAL32 HEADER al.h HEADER_SUFFIXES AL)

# Link only 
search_dependency(OPUS LIBRARY opus)
if (MSVC)
  search_dependency(PTHREAD LIBRARY pthreadVC2)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} ws2_32 iphlpapi)
else()
  set(ALL_LIBRARIES ${ALL_LIBRARIES} -pthread)
endif()

if (PLATFORM_EXTENSIONS AND UNIX AND NOT APPLE)
  # Automatic auto-away support. (X11 also using for capslock detection)
  search_dependency(X11               LIBRARY X11 OPTIONAL)
  search_dependency(XSS               LIBRARY Xss OPTIONAL)
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
  search_dependency(OPENSSL           LIBRARY libcrypto HEADER openssl/crypto.h)
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
    OUTPUT_STRIP_TRAILING_WHITESPACE
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

add_definitions(
  -DTIMESTAMP=${TIMESTAMP}
  -DLOG_TO_FILE=1
)
