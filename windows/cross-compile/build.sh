#!/usr/bin/env bash

# MIT License
#
# Copyright (c) 2017-2018 Maxim Biro <nurupo.contributions@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# Known issues:
# - Doesn't build qTox updater, because it wasn't ported to cmake yet and
#   because it requires static Qt, which means we'd need to build Qt twice, and
#   building Qt takes really long time.


set -euo pipefail


# Common directory paths

readonly WORKSPACE_DIR="/workspace"
readonly QTOX_SRC_DIR="/qtox"


# Make sure we run in an expected environment
if [ ! -f /etc/os-release ] || ! cat /etc/os-release | grep -qi 'buster'
then
  echo "Error: This script should be run on Debian Buster."
  exit 1
fi

if [ ! -d "$WORKSPACE_DIR" ] || [ ! -d "$QTOX_SRC_DIR" ]
then
  echo "Error: At least one of $WORKSPACE_DIR or $QTOX_SRC_DIR directories is missing."
  exit 1
fi

if [ ! -d "$QTOX_SRC_DIR/src" ]
then
  echo "Error: $QTOX_SRC_DIR/src directory is missing, make sure $QTOX_SRC_DIR contains qTox source code."
  exit 1
fi

if [[ "$(id -u)" != "0" ]]
then
  echo "Error: This script must be run as root."
  exit 1
fi


# Check arguments

readonly ARCH=$1
readonly BUILD_TYPE=$2

if [ -z "$ARCH" ]
then
  echo "Error: No architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the first argument to the script."
  exit 1
fi

if [[ "$ARCH" != "i686" ]] && [[ "$ARCH" != "x86_64" ]]
then
  echo "Error: Incorrect architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the first argument to the script."
  exit 1
fi

if [ -z "$BUILD_TYPE" ]
then
  echo "Error: No build type was specified. Please specify either 'release' or 'debug', case sensitive, as the second argument to the script."
  exit 1
fi

if [[ "$BUILD_TYPE" != "release" ]] && [[ "$BUILD_TYPE" != "debug" ]]
then
  echo "Error: Incorrect build type was specified. Please specify either 'release' or 'debug', case sensitive, as the second argument to the script."
  exit 1
fi


# More directory variables

readonly BUILD_DIR="/build"
readonly DEP_DIR="$WORKSPACE_DIR/$ARCH/dep-cache"
readonly APT_CACHE_DIR="$WORKSPACE_DIR/$ARCH/apt_cache"

# Create the expected directory structure

# Just make sure those exist
mkdir -p "$WORKSPACE_DIR"
mkdir -p "$DEP_DIR"
mkdir -p "$APT_CACHE_DIR"

# Build dir should be empty
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

set -x

echo "Restoring package cache"
# ensure at least one file exists
touch "$APT_CACHE_DIR"/dummy
# restore apt cache
cp -r "$APT_CACHE_DIR"/* /var/cache/

# remove docker specific config file, this file prevents usage of the package cache
rm -f /etc/apt/apt.conf.d/docker-clean

readonly WGET_OPTIONS="--timeout=10"

# Get packages

apt-get update
apt-get install -y --no-install-recommends \
                   autoconf \
                   automake \
                   build-essential \
                   bsdtar \
                   ca-certificates \
                   cmake \
                   git \
                   libtool \
                   nsis \
                   gettext \
                   pkg-config \
                   tclsh \
                   unzip \
                   wget \
                   yasm \
                   zip

if [[ "$ARCH" == "i686" ]]
then
  apt-get install -y --no-install-recommends \
                    g++-mingw-w64-i686 \
                    gcc-mingw-w64-i686
elif [[ "$ARCH" == "x86_64" ]]
then
  apt-get install -y --no-install-recommends \
                    g++-mingw-w64-x86-64 \
                    gcc-mingw-w64-x86-64
fi


# Install wine to run qTox tests in
set +u
if [[ "$TRAVIS_CI_STAGE" == "stage3" ]]
then
  dpkg --add-architecture i386
  apt-get update
  apt-get install -y wine wine32 wine64
fi
set -u


# Use all cores for building

MAKEFLAGS=j$(nproc)
export MAKEFLAGS


# Helper functions

# We check sha256 of all tarballs we download
check_sha256()
{
  if ! ( echo "$1  $2" | sha256sum -c --status - )
  then
    echo "Error: sha256 of $2 doesn't match the known one."
    echo "Expected: $1  $2"
    echo -n "Got: "
    sha256sum "$2"
    exit 1
  else
    echo "sha256 matches the expected one: $1"
  fi
}

# If it's not a tarball but a git repo, let's check a hash of a file containing hashes of all files
check_sha256_git()
{
  # There shoudl be .git directory
  if [ ! -d ".git" ]
  then
    echo "Error: this function should be called in the root of a git repository."
    exit 1
  fi
  # Create a file listing hashes of all the files except .git/*
  find . -type f | grep -v "^./.git" | LC_COLLATE=C sort --stable --ignore-case | xargs sha256sum > /tmp/hashes.sha
  check_sha256 "$1" "/tmp/hashes.sha"
}

# Strip binaries to reduce file size, we don't need this information anyway
strip_all()
{
  set +e
  for PREFIX_DIR in $DEP_DIR/*; do
    strip --strip-unneeded $PREFIX_DIR/bin/*
    $ARCH-w64-mingw32-strip --strip-unneeded $PREFIX_DIR/bin/*
    $ARCH-w64-mingw32-strip --strip-unneeded $PREFIX_DIR/lib/*
  done
  set -e
}

# Store apt cache
store_apt_cache()
{
  # prevent old packages from polluting the cache
  apt-get autoclean
  cp -r /var/cache/apt/ "$APT_CACHE_DIR"
}


# OpenSSL

OPENSSL_PREFIX_DIR="$DEP_DIR/libopenssl"
OPENSSL_VERSION=1.1.1g
# hash from https://www.openssl.org/source/
OPENSSL_HASH="ddb04774f1e32f0c49751e21b67216ac87852ceb056b75209af2443400636d46"
OPENSSL_FILENAME="openssl-$OPENSSL_VERSION.tar.gz"
if [ ! -f "$OPENSSL_PREFIX_DIR/done" ]
then
  rm -rf "$OPENSSL_PREFIX_DIR"
  mkdir -p "$OPENSSL_PREFIX_DIR"

  wget $WGET_OPTIONS "https://www.openssl.org/source/$OPENSSL_FILENAME"
  check_sha256 "$OPENSSL_HASH" "$OPENSSL_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$OPENSSL_FILENAME"
  rm $OPENSSL_FILENAME
  cd openssl*

  CONFIGURE_OPTIONS="--prefix=$OPENSSL_PREFIX_DIR --openssldir=${OPENSSL_PREFIX_DIR}/ssl shared"
  if [[ "$ARCH" == "x86_64" ]]
  then
    CONFIGURE_OPTIONS="$CONFIGURE_OPTIONS mingw64 --cross-compile-prefix=x86_64-w64-mingw32-"
  elif [[ "$ARCH" == "i686" ]]
  then
    CONFIGURE_OPTIONS="$CONFIGURE_OPTIONS mingw --cross-compile-prefix=i686-w64-mingw32-"
  fi

  ./Configure $CONFIGURE_OPTIONS
  make
  make install
  echo -n $OPENSSL_VERSION > $OPENSSL_PREFIX_DIR/done

  CONFIGURE_OPTIONS=""

  cd ..
  rm -rf ./openssl*
else
  echo "Using cached build of OpenSSL `cat $OPENSSL_PREFIX_DIR/done`"
fi


# Qt

QT_PREFIX_DIR="$DEP_DIR/libqt5"
QT_MAJOR=5
QT_MINOR=12
QT_PATCH=8
QT_VERSION=$QT_MAJOR.$QT_MINOR.$QT_PATCH
# hash from https://download.qt.io/archive/qt/5.12/5.12.8/single/qt-everywhere-src-5.12.8.tar.xz.mirrorlist
QT_HASH="9142300dfbd641ebdea853546511a352e4bd547c4c7f25d61a40cd997af1f0cf"
QT_FILENAME="qt-everywhere-src-$QT_VERSION.tar.xz"
if [ ! -f "$QT_PREFIX_DIR/done" ]
then
  rm -rf "$QT_PREFIX_DIR"
  mkdir -p "$QT_PREFIX_DIR"

  wget $WGET_OPTIONS "https://download.qt.io/official_releases/qt/$QT_MAJOR.$QT_MINOR/$QT_VERSION/single/$QT_FILENAME"
  check_sha256 "$QT_HASH" "$QT_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf $QT_FILENAME
  rm $QT_FILENAME
  cd qt*

  export PKG_CONFIG_PATH="$OPENSSL_PREFIX_DIR/lib/pkgconfig"
  export OPENSSL_LIBS="$(pkg-config --libs openssl)"

  # So, apparently Travis CI terminates a build if it generates more than 4mb of stdout output
  # which happens when building Qt
  CONFIGURE_EXTRA=""
  set +u
  if [[ "$TRAVIS_CI_STAGE" == "stage1" ]]
  then
    CONFIGURE_EXTRA="-silent"
  fi
  set -u

  ./configure -prefix $QT_PREFIX_DIR \
    -release \
    -shared \
    -device-option CROSS_COMPILE=$ARCH-w64-mingw32- \
    -xplatform win32-g++ \
    -openssl \
    $(pkg-config --cflags openssl) \
    -opensource -confirm-license \
    -pch \
    -nomake examples \
    -nomake tools \
    -nomake tests \
    -skip 3d \
    -skip activeqt \
    -skip androidextras \
    -skip canvas3d \
    -skip charts \
    -skip connectivity \
    -skip datavis3d \
    -skip declarative \
    -skip doc \
    -skip gamepad \
    -skip graphicaleffects \
    -skip imageformats \
    -skip location \
    -skip macextras \
    -skip multimedia \
    -skip networkauth \
    -skip purchasing \
    -skip quickcontrols \
    -skip quickcontrols2 \
    -skip remoteobjects \
    -skip script \
    -skip scxml \
    -skip sensors \
    -skip serialbus \
    -skip serialport \
    -skip speech \
    -skip translations \
    -skip virtualkeyboard \
    -skip wayland \
    -skip webchannel \
    -skip webengine \
    -skip webglplugin \
    -skip websockets \
    -skip webview \
    -skip x11extras \
    -skip xmlpatterns \
    -no-dbus \
    -no-icu \
    -no-compile-examples \
    -qt-libjpeg \
    -qt-libpng \
    -qt-zlib \
    -qt-pcre \
    -opengl desktop $CONFIGURE_EXTRA

  make
  make install
  echo -n $QT_VERSION > $QT_PREFIX_DIR/done

  unset PKG_CONFIG_PATH
  unset OPENSSL_LIBS

  cd ..
  rm -rf ./qt*
else
  echo "Using cached build of Qt `cat $QT_PREFIX_DIR/done`"
fi


# Stop here if running the first stage on Travis CI
set +u
if [[ "$TRAVIS_CI_STAGE" == "stage1" ]]
then
  # Strip to reduce cache size
  strip_all
  store_apt_cache
  # Chmod since everything is root:root
  chmod 777 -R "$WORKSPACE_DIR"
  exit 0
fi
set -u


# SQLCipher

SQLCIPHER_PREFIX_DIR="$DEP_DIR/libsqlcipher"
SQLCIPHER_VERSION=v4.4.0
SQLCIPHER_HASH="0924b2ae1079717954498bda78a30de20ce2a6083076b16214a711567821d148"
SQLCIPHER_FILENAME="$SQLCIPHER_VERSION.tar.gz"
if [ ! -f "$SQLCIPHER_PREFIX_DIR/done" ]
then
  rm -rf "$SQLCIPHER_PREFIX_DIR"
  mkdir -p "$SQLCIPHER_PREFIX_DIR"

  wget $WGET_OPTIONS "https://github.com/sqlcipher/sqlcipher/archive/$SQLCIPHER_FILENAME"
  check_sha256 "$SQLCIPHER_HASH" "$SQLCIPHER_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$SQLCIPHER_FILENAME"
  rm $SQLCIPHER_FILENAME
  cd sqlcipher*

  sed -i s/'if test "$TARGET_EXEEXT" = ".exe"'/'if test ".exe" = ".exe"'/g configure
  sed -i 's|exec $PWD/mksourceid manifest|exec $PWD/mksourceid.exe manifest|g' tool/mksqlite3h.tcl

  ./configure --host="$ARCH-w64-mingw32" \
              --prefix="$SQLCIPHER_PREFIX_DIR" \
              --disable-shared \
              --enable-tempstore=yes \
              CFLAGS="-O2 -g0 -DSQLITE_HAS_CODEC -I$OPENSSL_PREFIX_DIR/include/" \
              LDFLAGS="$OPENSSL_PREFIX_DIR/lib/libcrypto.a -lcrypto -lgdi32 -L$OPENSSL_PREFIX_DIR/lib/" \
              LIBS="-lgdi32 -lws2_32"

  sed -i s/"TEXE = $"/"TEXE = .exe"/ Makefile

  make
  make install
  echo -n $SQLCIPHER_VERSION > $SQLCIPHER_PREFIX_DIR/done

  cd ..
  rm -rf ./sqlcipher*
else
  echo "Using cached build of SQLCipher `cat $SQLCIPHER_PREFIX_DIR/done`"
fi


# FFmpeg

FFMPEG_PREFIX_DIR="$DEP_DIR/libffmpeg"
FFMPEG_VERSION=4.2.3
FFMPEG_HASH="9df6c90aed1337634c1fb026fb01c154c29c82a64ea71291ff2da9aacb9aad31"
FFMPEG_FILENAME="ffmpeg-$FFMPEG_VERSION.tar.xz"
if [ ! -f "$FFMPEG_PREFIX_DIR/done" ]
then
  rm -rf "$FFMPEG_PREFIX_DIR"
  mkdir -p "$FFMPEG_PREFIX_DIR"

  wget $WGET_OPTIONS "https://www.ffmpeg.org/releases/$FFMPEG_FILENAME"
  check_sha256 "$FFMPEG_HASH" "$FFMPEG_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf $FFMPEG_FILENAME
  rm $FFMPEG_FILENAME
  cd ffmpeg*

  if [[ "$ARCH" == "x86_64"* ]]
  then
    CONFIGURE_OPTIONS="--arch=x86_64"
  elif [[ "$ARCH" == "i686" ]]
  then
    CONFIGURE_OPTIONS="--arch=x86"
  fi

  ./configure $CONFIGURE_OPTIONS \
              --enable-gpl \
              --prefix="$FFMPEG_PREFIX_DIR" \
              --target-os="mingw32" \
              --cross-prefix="$ARCH-w64-mingw32-" \
              --pkg-config="pkg-config" \
              --extra-cflags="-static -O2 -g0" \
              --extra-ldflags="-lm -static" \
              --pkg-config-flags="--static" \
              --disable-debug \
              --disable-shared \
              --disable-programs \
              --disable-protocols \
              --disable-doc \
              --disable-sdl2 \
              --disable-avfilter \
              --disable-avresample \
              --disable-filters \
              --disable-iconv \
              --disable-network \
              --disable-muxers \
              --disable-postproc \
              --disable-swresample \
              --disable-swscale-alpha \
              --disable-dct \
              --disable-dwt \
              --disable-lsp \
              --disable-lzo \
              --disable-mdct \
              --disable-rdft \
              --disable-fft \
              --disable-faan \
              --disable-vaapi \
              --disable-vdpau \
              --disable-zlib \
              --disable-xlib \
              --disable-bzlib \
              --disable-lzma \
              --disable-encoders \
              --disable-decoders \
              --disable-demuxers \
              --disable-parsers \
              --disable-bsfs \
              --enable-demuxer=h264 \
              --enable-demuxer=mjpeg \
              --enable-parser=h264 \
              --enable-parser=mjpeg \
              --enable-decoder=h264 \
              --enable-decoder=mjpeg \
              --enable-decoder=rawvideo
  make
  make install
  echo -n $FFMPEG_VERSION > $FFMPEG_PREFIX_DIR/done

  CONFIGURE_OPTIONS=""

  cd ..
  rm -rf ./ffmpeg*
else
  echo "Using cached build of FFmpeg `cat $FFMPEG_PREFIX_DIR/done`"
fi


# Openal-soft (irungentoo's fork)
# We can stop using the fork once OpenAL-Soft gets loopback capture implemented:
# https://github.com/kcat/openal-soft/pull/421

OPENAL_PREFIX_DIR="$DEP_DIR/libopenal"
OPENAL_VERSION=b80570bed017de60b67c6452264c634085c3b148
OPENAL_HASH="734ef00895a9c1eb1505c11d638030b73593376df75da66ac5db6aa3e2f76807"
if [ ! -f "$OPENAL_PREFIX_DIR/done" ]
then
  rm -rf "$OPENAL_PREFIX_DIR"
  mkdir -p "$OPENAL_PREFIX_DIR"

  git clone https://github.com/irungentoo/openal-soft-tox openal-soft-tox
  cd openal*
  git checkout $OPENAL_VERSION
  check_sha256_git "$OPENAL_HASH"

# https://github.com/microsoft/vcpkg/blob/3baf583934f3077070e9ed4e7684f743ecced577/ports/openal-soft/cmake-3-11.patch
> cmake-3-11.patch cat << "EOF"
diff --git a/CMakeLists.txt b/CMakeLists.txt
index a871f4c..f9f6b34 100644
--- a/CMakeLists.txt
+++ b/CMakeLists.txt
@@ -965,7 +965,8 @@ OPTION(ALSOFT_REQUIRE_DSOUND "Require DirectSound backend" OFF)
 OPTION(ALSOFT_REQUIRE_MMDEVAPI "Require MMDevApi backend" OFF)
 IF(HAVE_WINDOWS_H)
     # Check MMSystem backend
-    CHECK_INCLUDE_FILES("windows.h;mmsystem.h" HAVE_MMSYSTEM_H -D_WIN32_WINNT=0x0502)
+    set(CMAKE_REQUIRED_DEFINITIONS -D_WIN32_WINNT=0x0502)
+    CHECK_INCLUDE_FILES("windows.h;mmsystem.h" HAVE_MMSYSTEM_H)
     IF(HAVE_MMSYSTEM_H)
         CHECK_SHARED_FUNCTION_EXISTS(waveOutOpen "windows.h;mmsystem.h" winmm "" HAVE_LIBWINMM)
         IF(HAVE_LIBWINMM)
EOF

  patch -l < cmake-3-11.patch

  mkdir -p build
  cd build

  echo "
      SET(CMAKE_SYSTEM_NAME Windows)

      SET(CMAKE_C_COMPILER $ARCH-w64-mingw32-gcc)
      SET(CMAKE_CXX_COMPILER $ARCH-w64-mingw32-g++)
      SET(CMAKE_RC_COMPILER $ARCH-w64-mingw32-windres)

      SET(CMAKE_FIND_ROOT_PATH /usr/$ARCH-w64-mingw32)

      # search for programs in the build host directories
      SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
      # for libraries and headers in the target directories
      SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
      SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  " > toolchain.cmake

  CFLAGS="-fPIC" cmake -DCMAKE_INSTALL_PREFIX="$OPENAL_PREFIX_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DALSOFT_UTILS=OFF \
    -DALSOFT_EXAMPLES=OFF \
    -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
    -DDSOUND_INCLUDE_DIR=/usr/$ARCH-w64-mingw32/include \
    -DDSOUND_LIBRARY=/usr/$ARCH-w64-mingw32/lib/libdsound.a \
    ..

  make
  make install
  echo -n $OPENAL_VERSION > $OPENAL_PREFIX_DIR/done

  cd ..

  cd ..
  rm -rf ./openal*
else
  echo "Using cached build of irungentoo's OpenAL-Soft fork `cat $OPENAL_PREFIX_DIR/done`"
fi


# QREncode

QRENCODE_PREFIX_DIR="$DEP_DIR/libqrencode"
QRENCODE_VERSION=4.0.2
QRENCODE_HASH="c9cb278d3b28dcc36b8d09e8cad51c0eca754eb004cb0247d4703cb4472b58b4"
QRENCODE_FILENAME="qrencode-$QRENCODE_VERSION.tar.bz2"
if [ ! -f "$QRENCODE_PREFIX_DIR/done" ]
then
  rm -rf "$QRENCODE_PREFIX_DIR"
  mkdir -p "$QRENCODE_PREFIX_DIR"

  wget $WGET_OPTIONS https://fukuchi.org/works/qrencode/$QRENCODE_FILENAME
  check_sha256 "$QRENCODE_HASH" "$QRENCODE_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$QRENCODE_FILENAME"
  rm $QRENCODE_FILENAME
  cd qrencode*

  CFLAGS="-O2 -g0" ./configure --host="$ARCH-w64-mingw32" \
                               --prefix="$QRENCODE_PREFIX_DIR" \
                               --disable-shared \
                               --enable-static \
                               --disable-sdltest \
                               --without-tools \
                               --without-debug
  make
  make install
  echo -n $QRENCODE_VERSION > $QRENCODE_PREFIX_DIR/done

  cd ..
  rm -rf ./qrencode*
else
  echo "Using cached build of QREncode `cat $QRENCODE_PREFIX_DIR/done`"
fi


# Exif

EXIF_PREFIX_DIR="$DEP_DIR/libexif"
EXIF_VERSION=0.6.22
EXIF_HASH="5048f1c8fc509cc636c2f97f4b40c293338b6041a5652082d5ee2cf54b530c56"
EXIF_FILENAME="libexif-$EXIF_VERSION.tar.xz"
if [ ! -f "$EXIF_PREFIX_DIR/done" ]
then
  rm -rf "$EXIF_PREFIX_DIR"
  mkdir -p "$EXIF_PREFIX_DIR"

  wget $WGET_OPTIONS "https://github.com/libexif/libexif/releases/download/libexif-${EXIF_VERSION//./_}-release/${EXIF_FILENAME}"
  check_sha256 "$EXIF_HASH" "$EXIF_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf $EXIF_FILENAME
  rm $EXIF_FILENAME
  cd libexif*

  CFLAGS="-O2 -g0" ./configure --host="$ARCH-w64-mingw32" \
                               --prefix="$EXIF_PREFIX_DIR" \
                               --disable-shared \
                               --enable-static \
                               --disable-docs \
                               --disable-nls
  make
  make install
  echo -n $EXIF_VERSION > $EXIF_PREFIX_DIR/done

  cd ..
  rm -rf ./libexif*
else
  echo "Using cached build of Exif `cat $EXIF_PREFIX_DIR/done`"
fi

# Hunspell

HUNSPELL_PREFIX_DIR="$DEP_DIR/Hunspell"
HUNSPELL_VERSION=1.7.0
HUNSPELL_DLL_VERSION=1.7-0
HUNSPELL_HASH="bb27b86eb910a8285407cf3ca33b62643a02798cf2eef468c0a74f6c3ee6bc8a"
HUNSPELL_FILENAME="v$HUNSPELL_VERSION.tar.gz"
if [ ! -f "$HUNSPELL_PREFIX_DIR/done" ]
then
  rm -rf "$HUNSPELL_PREFIX_DIR"
  mkdir -p "$HUNSPELL_PREFIX_DIR"

  wget "https://github.com/hunspell/hunspell/archive/v$HUNSPELL_VERSION.tar.gz"
  check_sha256 "$HUNSPELL_HASH" "$HUNSPELL_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$HUNSPELL_FILENAME"
  rm $HUNSPELL_FILENAME
  cd hunspell*
  autoreconf -vfi
  # -g makes hunspell not crash, but it's unclear why
  # https://github.com/hunspell/hunspell/issues/627
  ./configure --host="$ARCH-w64-mingw32" \
              --prefix=$HUNSPELL_PREFIX_DIR \
                CXXFLAGS='-g' \
              --disable-static \
              --enable-shared
  make
  make install
  ldconfig
  echo -n $HUNSPELL_VERSION > $HUNSPELL_PREFIX_DIR/done

  cd ..
  rm -rf ./hunspell*
else
  echo "Using cached build of Hunspell `cat $HUNSPELL_PREFIX_DIR/done`"
fi

# Dictionaries
DICTIONARY_PREFIX_DIR="$DEP_DIR/Dictionaries"

# EN_US
EN_US_PREFIX_DIR="$DICTIONARY_PREFIX_DIR/en_US/hunspell"
EN_US_HASH="17452350eeced560ab6f1300dfe1a15c52c884635839cca85ce0307fe4e2f5f2"
EN_US_VERSION="2019-03-01gb"
EN_US_FILENAME="$EN_US_VERSION.tar.gz"

if [ ! -f "$EN_US_PREFIX_DIR/done" ]
then
  rm -rf "$EN_US_PREFIX_DIR"
  mkdir -p "$EN_US_PREFIX_DIR"

  wget "https://github.com/marcoagpinto/aoo-mozilla-en-dict/archive/$EN_US_FILENAME"
  check_sha256 "$EN_US_HASH" "$EN_US_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$EN_US_FILENAME"
  rm $EN_US_FILENAME
  cd aoo-mozilla*/en_US*
  cp en_US.aff $EN_US_PREFIX_DIR
  cp en_US.dic $EN_US_PREFIX_DIR
  echo -n $EN_US_VERSION > $EN_US_PREFIX_DIR/done
  cd ../..
  rm -rf ./aoo-mozilla*
else
  echo "Using cached en_US dictionary `cat $EN_US_PREFIX_DIR/done`"
fi

# extra-cmake-modules
# version available in debian stretch repos is not new enough
ECM_PREFIX_DIR="$DEP_DIR/ECM"
KDE_FRAMEWORK_VERSION=5.62
ECM_VERSION=$KDE_FRAMEWORK_VERSION.0
ECM_HASH="e07acfecef1b4c7a481a253b58b75072a4f887376301108ed2c753b5002adcd4"
ECM_FILENAME="extra-cmake-modules-$ECM_VERSION.tar.xz"
if [ ! -f "$ECM_PREFIX_DIR/done" ]
then
  rm -rf "$ECM_PREFIX_DIR"
  mkdir -p "$ECM_PREFIX_DIR"

  wget "https://download.kde.org/stable/frameworks/$KDE_FRAMEWORK_VERSION/$ECM_FILENAME"
  check_sha256 "$ECM_HASH" "$ECM_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$ECM_FILENAME"
  rm $ECM_FILENAME
  cd extra-cmake-modules*
  mkdir build
  cd build

  echo "
      SET(CMAKE_SYSTEM_NAME Windows)
      SET(CMAKE_C_COMPILER $ARCH-w64-mingw32-gcc)
      SET(CMAKE_CXX_COMPILER $ARCH-w64-mingw32-g++)
      SET(CMAKE_RC_COMPILER $ARCH-w64-mingw32-windres)
      SET(CMAKE_FIND_ROOT_PATH /usr/$ARCH-w64-mingw32)
  " > toolchain.cmake

  cmake -DCMAKE_INSTALL_PREFIX=$ECM_PREFIX_DIR \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
        ..
  make
  make install
  cd ..

  echo -n $ECM_VERSION > $ECM_PREFIX_DIR/done

  cd ..
  rm -rf ./extra-cmake-modules*
else
  echo "Using cached build of extra-cmake-modules `cat $ECM_PREFIX_DIR/done`"
fi

# KF5Sonnet

KF5SONNET_PREFIX_DIR="$DEP_DIR/sonnet"
KF5SONNET_VERSION=$KDE_FRAMEWORK_VERSION.0
KF5SONNET_HASH="a1a2d3500d7fc51d94fd6f9d951c83be86436284aeda8416963fc5213956a69a"
KF5SONNET_FILENAME="sonnet-$KF5SONNET_VERSION.tar.xz"
if [ ! -f "$KF5SONNET_PREFIX_DIR/done" ]
then
  rm -rf "$KF5SONNET_PREFIX_DIR"
  mkdir -p "$KF5SONNET_PREFIX_DIR"

  wget "https://download.kde.org/stable/frameworks/$KDE_FRAMEWORK_VERSION/$KF5SONNET_FILENAME"
  check_sha256 "$KF5SONNET_HASH" "$KF5SONNET_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$KF5SONNET_FILENAME"
  rm $KF5SONNET_FILENAME
  cd sonnet*

  cd cmake
# Do not remove trailing whitespace and dont replace tabs with spaces in the patch below,
#  otherwise the patch will fail to apply
> FindHUNSPELL.cmake-patch cat << "EOF"
--- FindHUNSPELL.cmake
+++ FindHUNSPELL.cmake
@@ -40,7 +40,7 @@ find_path(HUNSPELL_INCLUDE_DIRS
           HINTS ${PKG_HUNSPELL_INCLUDE_DIRS}
 )
 find_library(HUNSPELL_LIBRARIES
-             NAMES ${PKG_HUNSPELL_LIBRARIES} hunspell hunspell-1.6 hunspell-1.5 hunspell-1.4 hunspell-1.3 hunspell-1.2 libhunspell
+             NAMES ${PKG_HUNSPELL_LIBRARIES} hunspell hunspell-1.7 hunspell-1.6 hunspell-1.5 hunspell-1.4 hunspell-1.3 hunspell-1.2 libhunspell
              HINTS ${PKG_HUNSPELL_LIBRARY_DIRS}
 )

EOF
  patch -l < FindHUNSPELL.cmake-patch
  cd ..

  mkdir build
  cd build
  echo "
      SET(CMAKE_SYSTEM_NAME Windows)
      SET(CMAKE_C_COMPILER $ARCH-w64-mingw32-gcc)
      SET(CMAKE_CXX_COMPILER $ARCH-w64-mingw32-g++)
      SET(CMAKE_RC_COMPILER $ARCH-w64-mingw32-windres)
      SET(CMAKE_FIND_ROOT_PATH /usr/$ARCH-w64-mingw32 $ECM_PREFIX_DIR $QT_PREFIX_DIR $HUNSPELL_PREFIX_DIR)
  " > toolchain.cmake

  cmake -DCMAKE_INSTALL_PREFIX=$KF5SONNET_PREFIX_DIR \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
        ..
  make
  make install
  cd ..

  echo -n $KF5SONNET_VERSION > $KF5SONNET_PREFIX_DIR/done

  cd ..
  # rm -rf ./sonnet*
else
  echo "Using cached build of KF5Sonnet `cat $KF5SONNET_PREFIX_DIR/done`"
fi

# Opus

OPUS_PREFIX_DIR="$DEP_DIR/libopus"
OPUS_VERSION=1.3.1
# https://archive.mozilla.org/pub/opus/SHA256SUMS.txt
OPUS_HASH="65b58e1e25b2a114157014736a3d9dfeaad8d41be1c8179866f144a2fb44ff9d"
OPUS_FILENAME="opus-$OPUS_VERSION.tar.gz"
if [ ! -f "$OPUS_PREFIX_DIR/done" ]
then
  rm -rf "$OPUS_PREFIX_DIR"
  mkdir -p "$OPUS_PREFIX_DIR"

  wget $WGET_OPTIONS "https://archive.mozilla.org/pub/opus/$OPUS_FILENAME"
  check_sha256 "$OPUS_HASH" "$OPUS_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$OPUS_FILENAME"
  rm $OPUS_FILENAME
  cd opus*

  CFLAGS="-O2 -g0" ./configure --host="$ARCH-w64-mingw32" \
                               --prefix="$OPUS_PREFIX_DIR" \
                               --disable-shared \
                               --enable-static \
                               --disable-extra-programs \
                               --disable-doc
  make
  make install
  echo -n $OPUS_VERSION > $OPUS_PREFIX_DIR/done

  cd ..
  rm -rf ./opus*
else
  echo "Using cached build of Opus `cat $OPUS_PREFIX_DIR/done`"
fi


# Sodium

SODIUM_PREFIX_DIR="$DEP_DIR/libsodium"
SODIUM_VERSION=1.0.18
SODIUM_HASH="6f504490b342a4f8a4c4a02fc9b866cbef8622d5df4e5452b46be121e46636c1"
SODIUM_FILENAME="libsodium-$SODIUM_VERSION.tar.gz"
if [ ! -f "$SODIUM_PREFIX_DIR/done" ]
then
  rm -rf "$SODIUM_PREFIX_DIR"
  mkdir -p "$SODIUM_PREFIX_DIR"

  wget $WGET_OPTIONS "https://download.libsodium.org/libsodium/releases/$SODIUM_FILENAME"
  check_sha256 "$SODIUM_HASH" "$SODIUM_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$SODIUM_FILENAME"
  rm "$SODIUM_FILENAME"
  cd libsodium*

  ./configure --host="$ARCH-w64-mingw32" \
              --prefix="$SODIUM_PREFIX_DIR" \
              --disable-shared \
              --enable-static \
              --with-pic
  make
  make install
  echo -n $SODIUM_VERSION > $SODIUM_PREFIX_DIR/done

  cd ..
  rm -rf ./libsodium*
else
  echo "Using cached build of Sodium `cat $SODIUM_PREFIX_DIR/done`"
fi


# VPX

VPX_PREFIX_DIR="$DEP_DIR/libvpx"
VPX_VERSION=v1.8.2
VPX_HASH="8735d9fcd1a781ae6917f28f239a8aa358ce4864ba113ea18af4bb2dc8b474ac"
VPX_FILENAME="libvpx-$VPX_VERSION.tar.gz"
if [ ! -f "$VPX_PREFIX_DIR/done" ]
then
  rm -rf "$VPX_PREFIX_DIR"
  mkdir -p "$VPX_PREFIX_DIR"

  wget $WGET_OPTIONS https://github.com/webmproject/libvpx/archive/$VPX_VERSION.tar.gz -O $VPX_FILENAME
  check_sha256 "$VPX_HASH" "$VPX_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$VPX_FILENAME"
  rm $VPX_FILENAME
  cd libvpx*

  if [[ "$ARCH" == "x86_64" ]]
  then
    VPX_TARGET=x86_64-win64-gcc
    # There is a bug in gcc that breaks avx512 on 64-bit Windows https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
    # VPX fails to build due to it.
    # This is a workaround as suggested in https://stackoverflow.com/questions/43152633
    VPX_CFLAGS="-fno-asynchronous-unwind-tables"
  elif [[ "$ARCH" == "i686" ]]
  then
    VPX_TARGET=x86-win32-gcc
    VPX_CFLAGS=""
  fi

  CFLAGS="$VPX_CFLAGS" \
  CROSS="$ARCH-w64-mingw32-" ./configure --target="$VPX_TARGET" \
                                         --prefix="$VPX_PREFIX_DIR" \
                                         --disable-shared \
                                         --enable-static \
                                         --disable-examples \
                                         --disable-tools \
                                         --disable-docs \
                                         --disable-unit-tests
  make
  make install
  echo -n $VPX_VERSION > $VPX_PREFIX_DIR/done

  cd ..
  rm -rf ./libvpx*
else
  echo "Using cached build of VPX `cat $VPX_PREFIX_DIR/done`"
fi


# Toxcore

TOXCORE_PREFIX_DIR="$DEP_DIR/libtoxcore"
TOXCORE_VERSION=0.2.11
TOXCORE_HASH="f111285b036d7746ce8d1321cf0b89ec93b4fad8ae90767a24e50230bbee27e1"
TOXCORE_FILENAME="c-toxcore-$TOXCORE_VERSION.tar.gz"
if [ ! -f "$TOXCORE_PREFIX_DIR/done" ]
then
  rm -rf "$TOXCORE_PREFIX_DIR"
  mkdir -p "$TOXCORE_PREFIX_DIR"

  wget $WGET_OPTIONS https://github.com/TokTok/c-toxcore/archive/v$TOXCORE_VERSION.tar.gz -O $TOXCORE_FILENAME
  check_sha256 "$TOXCORE_HASH" "$TOXCORE_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$TOXCORE_FILENAME"
  rm "$TOXCORE_FILENAME"
  cd c-toxcore*

  mkdir -p build
  cd build

  export PKG_CONFIG_PATH="$OPUS_PREFIX_DIR/lib/pkgconfig:$SODIUM_PREFIX_DIR/lib/pkgconfig:$VPX_PREFIX_DIR/lib/pkgconfig"
  export PKG_CONFIG_LIBDIR="/usr/$ARCH-w64-mingw32"

  echo "
      SET(CMAKE_SYSTEM_NAME Windows)

      SET(CMAKE_C_COMPILER $ARCH-w64-mingw32-gcc)
      SET(CMAKE_CXX_COMPILER $ARCH-w64-mingw32-g++)
      SET(CMAKE_RC_COMPILER $ARCH-w64-mingw32-windres)

      SET(CMAKE_FIND_ROOT_PATH /usr/$ARCH-w64-mingw32 $OPUS_PREFIX_DIR $SODIUM_PREFIX_DIR $VPX_PREFIX_DIR)
  " > toolchain.cmake

  cmake -DCMAKE_INSTALL_PREFIX=$TOXCORE_PREFIX_DIR \
        -DBOOTSTRAP_DAEMON=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DENABLE_STATIC=ON \
        -DENABLE_SHARED=OFF \
        -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
        ..

  make
  make install
  echo -n $TOXCORE_VERSION > $TOXCORE_PREFIX_DIR/done

  unset PKG_CONFIG_PATH
  unset PKG_CONFIG_LIBDIR

  cd ..

  cd ..
  rm -rf ./c-toxcore*
else
  echo "Using cached build of Toxcore `cat $TOXCORE_PREFIX_DIR/done`"
fi


# mingw-w64-debug-scripts

MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR="$DEP_DIR/mingw-w64-debug-scripts"
MINGW_W64_DEBUG_SCRIPTS_VERSION=7341e1ffdea352e5557f3fcae51569f13e1ef270
MINGW_W64_DEBUG_SCRIPTS_HASH="a92883ddfe83780818347fda4ac07bce61df9226818df2f52fe4398fe733e204"
if [ ! -f "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/done" ]
then
  rm -rf "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR"
  mkdir -p "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR"

  # Get dbg executable and the debug scripts
  git clone https://github.com/nurupo/mingw-w64-debug-scripts mingw-w64-debug-scripts
  cd mingw-w64-debug-scripts
  git checkout $MINGW_W64_DEBUG_SCRIPTS_VERSION
  check_sha256_git "$MINGW_W64_DEBUG_SCRIPTS_HASH"

  make $ARCH EXE_NAME=qtox.exe
  mkdir -p "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/bin"
  mv output/$ARCH/* "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/bin/"
  echo -n $MINGW_W64_DEBUG_SCRIPTS_VERSION > $MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/done

  cd ..
  rm -rf ./mingw-w64-debug-scripts
else
  echo "Using cached build of mingw-w64-debug-scripts `cat $MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/done`"
fi


# NSIS ShellExecAsUser plugin

NSISSHELLEXECASUSER_PREFIX_DIR="$DEP_DIR/nsis_shellexecuteasuser"
NSISSHELLEXECASUSER_VERSION=" "
NSISSHELLEXECASUSER_HASH="8fc19829e144716a422b15a85e718e1816fe561de379b2b5ae87ef9017490799"
if [ ! -f "$NSISSHELLEXECASUSER_PREFIX_DIR/done" ]
then
  rm -rf "$NSISSHELLEXECASUSER_PREFIX_DIR"
  mkdir -p "$NSISSHELLEXECASUSER_PREFIX_DIR"

  # Backup: https://web.archive.org/web/20171008011417/http://nsis.sourceforge.net/mediawiki/images/c/c7/ShellExecAsUser.zip
  wget $WGET_OPTIONS http://nsis.sourceforge.net/mediawiki/images/c/c7/ShellExecAsUser.zip
  check_sha256 "$NSISSHELLEXECASUSER_HASH" "ShellExecAsUser.zip"
  unzip ShellExecAsUser.zip 'ShellExecAsUser.dll'

  mkdir "$NSISSHELLEXECASUSER_PREFIX_DIR/bin"
  mv ShellExecAsUser.dll "$NSISSHELLEXECASUSER_PREFIX_DIR/bin"
  rm ShellExecAsUser*
  echo -n $NSISSHELLEXECASUSER_VERSION > $NSISSHELLEXECASUSER_PREFIX_DIR/done
else
  echo "Using cached build of NSIS ShellExecAsUser plugin `cat $NSISSHELLEXECASUSER_PREFIX_DIR/done`"
fi
# Install ShellExecAsUser plugin
cp "$NSISSHELLEXECASUSER_PREFIX_DIR/bin/ShellExecAsUser.dll" /usr/share/nsis/Plugins/x86-ansi/


# Stop here if running the second stage on Travis CI
set +u
if [[ "$TRAVIS_CI_STAGE" == "stage2" ]]
then
  # Strip to reduce cache size
  strip_all
  store_apt_cache
  # Chmod since everything is root:root
  chmod 777 -R "$WORKSPACE_DIR"
  exit 0
fi
set -u


strip_all


# qTox

QTOX_PREFIX_DIR="$WORKSPACE_DIR/$ARCH/qtox/$BUILD_TYPE"
rm -rf "$QTOX_PREFIX_DIR"
mkdir -p "$QTOX_PREFIX_DIR"

rm -rf ./qtox
mkdir -p qtox
cd qtox
cp -a $QTOX_SRC_DIR/. .

rm -rf ./build
mkdir -p build
cd build

PKG_CONFIG_PATH=""
PKG_CONFIG_LIBDIR=""
CMAKE_FIND_ROOT_PATH=""
for PREFIX_DIR in $DEP_DIR/*; do
  if [ -d $PREFIX_DIR/lib/pkgconfig ]
  then
    export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$PREFIX_DIR/lib/pkgconfig"
    export PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR:$PREFIX_DIR/lib/pkgconfig"
  fi
  CMAKE_FIND_ROOT_PATH="$CMAKE_FIND_ROOT_PATH $PREFIX_DIR"
done

echo "
    SET(CMAKE_SYSTEM_NAME Windows)

    SET(CMAKE_C_COMPILER $ARCH-w64-mingw32-gcc)
    SET(CMAKE_CXX_COMPILER $ARCH-w64-mingw32-g++)
    SET(CMAKE_RC_COMPILER $ARCH-w64-mingw32-windres)

    SET(CMAKE_FIND_ROOT_PATH /usr/$ARCH-w64-mingw32 $CMAKE_FIND_ROOT_PATH)
" > toolchain.cmake

set +u
if [[ "$TRAVIS_CI_STAGE" == "stage3" ]]
then
  echo "SET(TEST_CROSSCOMPILING_EMULATOR /usr/bin/wine)" >> toolchain.cmake
fi
set -u

if [[ "$BUILD_TYPE" == "release" ]]
then
  cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DUPDATE_CHECK=ON \
        -DSTRICT_OPTIONS=ON \
        ..
elif [[ "$BUILD_TYPE" == "debug" ]]
then
  cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Debug \
        -DUPDATE_CHECK=ON \
        -DSTRICT_OPTIONS=ON \
        ..
fi

make

cp qtox.exe $QTOX_PREFIX_DIR
cp $QT_PREFIX_DIR/bin/Qt5Core.dll \
   $QT_PREFIX_DIR/bin/Qt5Gui.dll \
   $QT_PREFIX_DIR/bin/Qt5Network.dll \
   $QT_PREFIX_DIR/bin/Qt5Svg.dll \
   $QT_PREFIX_DIR/bin/Qt5Widgets.dll \
   $QT_PREFIX_DIR/bin/Qt5Xml.dll \
   $QTOX_PREFIX_DIR
cp -r $QT_PREFIX_DIR/plugins/imageformats \
      $QT_PREFIX_DIR/plugins/platforms \
      $QT_PREFIX_DIR/plugins/iconengines \
      $QTOX_PREFIX_DIR
cp $OPENAL_PREFIX_DIR/bin/OpenAL32.dll $QTOX_PREFIX_DIR
cp $OPENSSL_PREFIX_DIR/bin/libssl-*.dll \
   $OPENSSL_PREFIX_DIR/bin/libcrypto-*.dll \
   $QTOX_PREFIX_DIR
cp $HUNSPELL_PREFIX_DIR/bin/libhunspell-$HUNSPELL_DLL_VERSION.dll $QTOX_PREFIX_DIR
cp $KF5SONNET_PREFIX_DIR/bin/libKF5SonnetUi.dll \
   $KF5SONNET_PREFIX_DIR/bin/libKF5SonnetCore.dll \
   $QTOX_PREFIX_DIR
cp -r $KF5SONNET_PREFIX_DIR/lib/plugins/kf5 $QTOX_PREFIX_DIR
cp -r $DICTIONARY_PREFIX_DIR/en_US/hunspell $QTOX_PREFIX_DIR
cp /usr/lib/gcc/$ARCH-w64-mingw32/*-posix/libgcc_s_*.dll $QTOX_PREFIX_DIR
cp /usr/lib/gcc/$ARCH-w64-mingw32/*-posix/libstdc++-6.dll $QTOX_PREFIX_DIR
cp /usr/$ARCH-w64-mingw32/lib/libwinpthread-1.dll $QTOX_PREFIX_DIR

set +u
if [[ "$TRAVIS_CI_STAGE" == "stage3" ]]
then
  # Setup wine
  if [[ "$ARCH" == "i686" ]]
  then
    export WINEARCH=win32
  elif [[ "$ARCH" == "x86_64" ]]
  then
    export WINEARCH=win64
  fi
  winecfg
  # Add libgcc_s_*.dll, libwinpthread-1.dll, QtTest.dll, etc. into PATH env var of wine
  export WINEPATH=`cd $QTOX_PREFIX_DIR ; winepath -w $(pwd)`\;`winepath -w $QT_PREFIX_DIR/bin/`
  export CTEST_OUTPUT_ON_FAILURE=1
  make test
fi
set -u

cd ..

# Setup gdb
if [[ "$BUILD_TYPE" == "debug" ]]
then
  # Copy over qTox source code so that dbg could pick it up
  mkdir -p "$QTOX_PREFIX_DIR/$PWD/src"
  cp -r "$PWD/src" "$QTOX_PREFIX_DIR/$PWD"

  # Get dbg executable and the debug scripts
  cp -r $MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/bin/* "$QTOX_PREFIX_DIR/"
fi

# Strip
set +e
if [[ "$BUILD_TYPE" == "release" ]]
then
  $ARCH-w64-mingw32-strip -s $QTOX_PREFIX_DIR/*.exe
fi
$ARCH-w64-mingw32-strip -s $QTOX_PREFIX_DIR/*.dll
$ARCH-w64-mingw32-strip -s $QTOX_PREFIX_DIR/*/*.dll
set -e

# Create zip
cd $QTOX_PREFIX_DIR
zip qtox-"$ARCH"-"$BUILD_TYPE".zip -r *
cd -

# Create installer
if [[ "$BUILD_TYPE" == "release" ]]
then
  cd windows

  # The installer creation script expects all the files to be in qtox/*
  mkdir qtox
  cp -r $QTOX_PREFIX_DIR/* ./qtox
  rm ./qtox/*.zip

  # Select the installer script for the correct architecture
  if [[ "$ARCH" == "i686" ]]
  then
    makensis qtox.nsi
  elif [[ "$ARCH" == "x86_64" ]]
  then
    makensis qtox64.nsi
  fi

  cp setup-qtox.exe $QTOX_PREFIX_DIR/setup-qtox-"$ARCH"-"$BUILD_TYPE".exe
  cd ..
fi

cd ..
rm -rf ./qtox

# Cache APT packages for future runs
store_apt_cache

# Chmod since everything is root:root
chmod 777 -R "$WORKSPACE_DIR"
