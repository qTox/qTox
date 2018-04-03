execute_process(COMMAND
            git describe
        WORKING_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE
            RES
        OUTPUT_VARIABLE
            GVERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT RES EQUAL 0)
    set(GVERSION "v0.0.0-NOTFOUND")
endif()

string(REPLACE "v" "" GVERSION "${GVERSION}")
string(REPLACE "-" ";" GVERSION "${GVERSION}")
string(REPLACE "." ";" VERSION_LIST "${GVERSION}")

list(GET VERSION_LIST 0 MAJOR)
list(GET VERSION_LIST 1 MINOR)
list(GET VERSION_LIST 2 PATCH)

list(LENGTH VERSION_LIST VSIZE)

if(${VSIZE} EQUAL 3)
    set(CPACK_PACKAGE_NAME "qtox")
else()
    set(CPACK_PACKAGE_NAME "qtox-nightly")
    set(BUILD_NUM $ENV{TRAVIS_BUILD_NUMBER})
    if(BUILD_NUM)
        set(PATCH "${PATCH}.${BUILD_NUM}")
    endif()
endif()

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH}")
set(CPACK_PACKAGE_CONTACT "qtox-dev@lists.tox.chat")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "ffmpeg, \
libappindicator1 (>= 0.2.96), \
libavcodec-ffmpeg56 (>= 7:2.4) | libavcodec-ffmpeg-extra56 (>= 7:2.4) |\
 libavcodec56 | libavcodec-ffmpeg-dev | libavcodec-dev, \
libavdevice-ffmpeg56 (>= 7:2.4) | libavdevice-ffmpeg-extra56 |\
  libavdevice56 | libavdevice-ffmpeg-dev | libavdevice-dev, \
libavformat-ffmpeg56 (>= 7:2.4) | libavformat-ffmpeg-extra56 (>= 7:2.4) |\
 libavformat56 | libavformat-ffmpeg-dev | libavformat-dev, \
libavutil-ffmpeg56 (>= 7:2.4) | libavutil-ffmpeg-extra56 (>= 7:2.4) |\
 libavutil56 | libavutil-ffmpeg-dev | libavutil-dev, \
libc6 (>= 2.17),
libcairo2 (>= 1.2.4),
libgdk-pixbuf2.0-0 (>= 2.22.0),
libglib2.0-0 (>= 2.37.3),
libgtk2.0-0 (>= 2.16.0),
libopenal1 (>= 1.14),
libopus0 (>= 1.1),
libqrencode3 (>= 3.2.0),
libqt5core5a (>= 5.5.0),
libqt5gui5 (>= 5.3.0) | libqt5gui5-gles (>= 5.3.0),
libqt5network5 (>= 5.0.2),
libqt5sql5 (>= 5.0.2),
libqt5svg5 (>= 5.0.2),
libqt5widgets5 (>= 5.3.0),
libqt5xml5 (>= 5.0.2),
libsodium18 (>= 0.6.1),
libsqlcipher0 (>= 1.1.9),
libstdc++6 (>= 5.2),
libswscale-ffmpeg3 (>= 7:2.4),
libvpx3 (>= 1.5.0),
libx11-6,
libxss1,
libexif12")

include(CPack)

