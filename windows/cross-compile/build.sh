#!/usr/bin/env bash

# MIT License
#
# Copyright (c) 2017-2020 Maxim Biro <nurupo.contributions@gmail.com>
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

readonly CURL_OPTIONS="-L --connect-timeout 10"

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
                   pkg-config \
                   python3-pefile \
                   tclsh \
                   texinfo \
                   unzip \
                   curl \
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
if [ -z "$TRAVIS_CI_STAGE" ] || [[ "$TRAVIS_CI_STAGE" == "stage3" ]]
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
  find . -type f | grep -v "^./.git" | LC_COLLATE=C sort --stable --ignore-case | xargs sha256sum > "/tmp/hashes-$1.sha"
  check_sha256 "$1" "/tmp/hashes-$1.sha"
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
OPENSSL_VERSION=1.1.1h
# hash from https://www.openssl.org/source/
OPENSSL_HASH="5c9ca8774bd7b03e5784f26ae9e9e6d749c9da2438545077e6b3d755a06595d9"
OPENSSL_FILENAME="openssl-$OPENSSL_VERSION.tar.gz"
if [ ! -f "$OPENSSL_PREFIX_DIR/done" ]
then
  rm -rf "$OPENSSL_PREFIX_DIR"
  mkdir -p "$OPENSSL_PREFIX_DIR"

  curl $CURL_OPTIONS -O "https://www.openssl.org/source/$OPENSSL_FILENAME"
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

  curl $CURL_OPTIONS -O "https://download.qt.io/official_releases/qt/$QT_MAJOR.$QT_MINOR/$QT_VERSION/single/$QT_FILENAME"
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
  if [[ -n "$TRAVIS_CI_STAGE" ]]
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

  curl $CURL_OPTIONS -O "https://github.com/sqlcipher/sqlcipher/archive/$SQLCIPHER_FILENAME"
  check_sha256 "$SQLCIPHER_HASH" "$SQLCIPHER_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$SQLCIPHER_FILENAME"
  rm $SQLCIPHER_FILENAME
  cd sqlcipher*

  sed -i s/'if test "$TARGET_EXEEXT" = ".exe"'/'if test ".exe" = ".exe"'/g configure
  sed -i 's|exec $PWD/mksourceid manifest|exec $PWD/mksourceid.exe manifest|g' tool/mksqlite3h.tcl

  ./configure --host="$ARCH-w64-mingw32" \
              --prefix="$SQLCIPHER_PREFIX_DIR" \
              --enable-shared \
              --disable-static \
              --enable-tempstore=yes \
              CFLAGS="-O2 -g0 -DSQLITE_HAS_CODEC -I$OPENSSL_PREFIX_DIR/include/" \
              LDFLAGS="-lcrypto -lgdi32 -L$OPENSSL_PREFIX_DIR/lib/" \
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

  curl $CURL_OPTIONS -O "https://www.ffmpeg.org/releases/$FFMPEG_FILENAME"
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
              --enable-shared \
              --disable-static \
              --prefix="$FFMPEG_PREFIX_DIR" \
              --target-os="mingw32" \
              --cross-prefix="$ARCH-w64-mingw32-" \
              --pkg-config="pkg-config" \
              --extra-cflags="-O2 -g0" \
              --disable-debug \
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

  curl $CURL_OPTIONS -O https://fukuchi.org/works/qrencode/$QRENCODE_FILENAME
  check_sha256 "$QRENCODE_HASH" "$QRENCODE_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$QRENCODE_FILENAME"
  rm $QRENCODE_FILENAME
  cd qrencode*

  CFLAGS="-O2 -g0" ./configure --host="$ARCH-w64-mingw32" \
                               --prefix="$QRENCODE_PREFIX_DIR" \
                               --enable-shared \
                               --disable-static \
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

  curl $CURL_OPTIONS -O "https://github.com/libexif/libexif/releases/download/libexif-${EXIF_VERSION//./_}-release/${EXIF_FILENAME}"
  check_sha256 "$EXIF_HASH" "$EXIF_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf $EXIF_FILENAME
  rm $EXIF_FILENAME
  cd libexif*

  CFLAGS="-O2 -g0" ./configure --host="$ARCH-w64-mingw32" \
                               --prefix="$EXIF_PREFIX_DIR" \
                               --enable-shared \
                               --disable-static \
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

  curl $CURL_OPTIONS -O "https://archive.mozilla.org/pub/opus/$OPUS_FILENAME"
  check_sha256 "$OPUS_HASH" "$OPUS_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$OPUS_FILENAME"
  rm $OPUS_FILENAME
  cd opus*

  CFLAGS="-O2 -g0" ./configure --host="$ARCH-w64-mingw32" \
                               --prefix="$OPUS_PREFIX_DIR" \
                               --enable-shared \
                               --disable-static \
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

  curl $CURL_OPTIONS -O "https://download.libsodium.org/libsodium/releases/$SODIUM_FILENAME"
  check_sha256 "$SODIUM_HASH" "$SODIUM_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$SODIUM_FILENAME"
  rm "$SODIUM_FILENAME"
  cd libsodium*

  ./configure --host="$ARCH-w64-mingw32" \
              --prefix="$SODIUM_PREFIX_DIR" \
              --enable-shared \
              --disable-static
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

  curl $CURL_OPTIONS https://github.com/webmproject/libvpx/archive/$VPX_VERSION.tar.gz -o $VPX_FILENAME
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

  cd ..

# Fix VPX not supporting creation of dll
# Modified version of https://aur.archlinux.org/cgit/aur.git/tree/configure.patch?h=mingw-w64-libvpx&id=6d658aa0f4d8409fcbd0f20be2c0adcf1e81a297
> configure.patch cat << "EOF"
diff -ruN libvpx/build/make/configure.sh patched/build/make/configure.sh
--- libvpx/build/make/configure.sh	2019-02-13 16:56:48.972857636 +0100
+++ patched/build/make/configure.sh	2019-02-13 16:50:37.995967583 +0100
@@ -1426,11 +1426,13 @@
         win32)
           add_asflags -f win32
           enabled debug && add_asflags -g cv8
+          add_ldflags "-Wl,-no-undefined"
           EXE_SFX=.exe
           ;;
         win64)
           add_asflags -f win64
           enabled debug && add_asflags -g cv8
+          add_ldflags "-Wl,-no-undefined"
           EXE_SFX=.exe
           ;;
         linux*|solaris*|android*)
diff -ruN libvpx/build/make/Makefile patched/build/make/Makefile
--- libvpx/build/make/Makefile	2019-02-13 16:56:48.972857636 +0100
+++ patched/build/make/Makefile	2019-02-13 16:50:37.995967583 +0100
@@ -304,6 +304,7 @@
 	$(if $(quiet),@echo "    [LD] $$@")
 	$(qexec)$$(LD) -shared $$(LDFLAGS) \
             -Wl,--no-undefined -Wl,-soname,$$(SONAME) \
+            -Wl,-out-implib,libvpx.dll.a \
             -Wl,--version-script,$$(EXPORTS_FILE) -o $$@ \
             $$(filter %.o,$$^) $$(extralibs)
 endef
@@ -388,7 +389,7 @@
 .libs: $(LIBS)
 	@touch $@
 $(foreach lib,$(filter %_g.a,$(LIBS)),$(eval $(call archive_template,$(lib))))
-$(foreach lib,$(filter %so.$(SO_VERSION_MAJOR).$(SO_VERSION_MINOR).$(SO_VERSION_PATCH),$(LIBS)),$(eval $(call so_template,$(lib))))
+$(foreach lib,$(filter %dll,$(LIBS)),$(eval $(call so_template,$(lib))))
 $(foreach lib,$(filter %$(SO_VERSION_MAJOR).dylib,$(LIBS)),$(eval $(call dl_template,$(lib))))
 $(foreach lib,$(filter %$(SO_VERSION_MAJOR).dll,$(LIBS)),$(eval $(call dll_template,$(lib))))
 
diff -ruN libvpx/configure patched/configure
--- libvpx/configure	2019-02-13 16:56:49.162860897 +0100
+++ patched/configure	2019-02-13 16:53:03.328719607 +0100
@@ -513,23 +513,23 @@
 }
 
 process_detect() {
-    if enabled shared; then
+    #if enabled shared; then
         # Can only build shared libs on a subset of platforms. Doing this check
         # here rather than at option parse time because the target auto-detect
         # magic happens after the command line has been parsed.
-        case "${tgt_os}" in
-        linux|os2|solaris|darwin*|iphonesimulator*)
+    #    case "${tgt_os}" in
+    #    linux|os2|solaris|darwin*|iphonesimulator*)
             # Supported platforms
-            ;;
-        *)
-            if enabled gnu; then
-                echo "--enable-shared is only supported on ELF; assuming this is OK"
-            else
-                die "--enable-shared only supported on ELF, OS/2, and Darwin for now"
-            fi
-            ;;
-        esac
-    fi
+    #        ;;
+    #    *)
+    #        if enabled gnu; then
+    #            echo "--enable-shared is only supported on ELF; assuming this is OK"
+    #        else
+    #            die "--enable-shared only supported on ELF, OS/2, and Darwin for now"
+    #        fi
+    #        ;;
+    #    esac
+    #fi
     if [ -z "$CC" ] || enabled external_build; then
         echo "Bypassing toolchain for environment detection."
         enable_feature external_build
diff -ruN libvpx/examples.mk patched/examples.mk
--- libvpx/examples.mk	2019-02-13 16:56:49.162860897 +0100
+++ patched/examples.mk	2019-02-13 16:50:37.995967583 +0100
@@ -315,7 +315,7 @@
 ifneq ($(filter os2%,$(TGT_OS)),)
 SHARED_LIB_SUF=_dll.a
 else
-SHARED_LIB_SUF=.so
+SHARED_LIB_SUF=.dll.a
 endif
 endif
 CODEC_LIB_SUF=$(if $(CONFIG_SHARED),$(SHARED_LIB_SUF),.a)
diff -ruN libvpx/libs.mk patched/libs.mk
--- libvpx/libs.mk	2019-02-13 16:56:48.972857636 +0100
+++ patched/libs.mk	2019-02-13 16:50:37.995967583 +0100
@@ -256,12 +256,12 @@
 LIBVPX_SO_SYMLINKS      :=
 LIBVPX_SO_IMPLIB        := libvpx_dll.a
 else
-LIBVPX_SO               := libvpx.so.$(SO_VERSION_MAJOR).$(SO_VERSION_MINOR).$(SO_VERSION_PATCH)
-SHARED_LIB_SUF          := .so
+LIBVPX_SO               := libvpx.dll
+SHARED_LIB_SUF          := .dll
 EXPORT_FILE             := libvpx.ver
-LIBVPX_SO_SYMLINKS      := $(addprefix $(LIBSUBDIR)/, \
-                             libvpx.so libvpx.so.$(SO_VERSION_MAJOR) \
-                             libvpx.so.$(SO_VERSION_MAJOR).$(SO_VERSION_MINOR))
+LIBVPX_SO_SYMLINKS      :=
+
+
 endif
 endif
 endif
@@ -271,7 +271,7 @@
                            $(if $(LIBVPX_SO_IMPLIB), $(BUILD_PFX)$(LIBVPX_SO_IMPLIB))
 $(BUILD_PFX)$(LIBVPX_SO): $(LIBVPX_OBJS) $(EXPORT_FILE)
 $(BUILD_PFX)$(LIBVPX_SO): extralibs += -lm
-$(BUILD_PFX)$(LIBVPX_SO): SONAME = libvpx.so.$(SO_VERSION_MAJOR)
+$(BUILD_PFX)$(LIBVPX_SO): SONAME = libvpx.dll
 $(BUILD_PFX)$(LIBVPX_SO): EXPORTS_FILE = $(EXPORT_FILE)
 
 libvpx.def: $(call enabled,CODEC_EXPORTS)
EOF

  cd -
  patch -Np1 < ../configure.patch
  rm ../configure.patch

  CFLAGS="$VPX_CFLAGS" \
  CROSS="$ARCH-w64-mingw32-" ./configure --target="$VPX_TARGET" \
                                         --prefix="$VPX_PREFIX_DIR" \
                                         --enable-shared \
                                         --disable-static \
                                         --enable-runtime-cpu-detect \
                                         --disable-examples \
                                         --disable-tools \
                                         --disable-docs \
                                         --disable-unit-tests
  make
  make install

  mkdir -p "$VPX_PREFIX_DIR/bin"
  mv "$VPX_PREFIX_DIR/lib/"*.dll "$VPX_PREFIX_DIR/bin/"
  mv ./libvpx*.dll.a "$VPX_PREFIX_DIR/lib/"

  echo -n $VPX_VERSION > $VPX_PREFIX_DIR/done

  cd ..
  rm -rf ./libvpx*
else
  echo "Using cached build of VPX `cat $VPX_PREFIX_DIR/done`"
fi


# Toxcore

TOXCORE_PREFIX_DIR="$DEP_DIR/libtoxcore"
TOXCORE_VERSION=0.2.12
TOXCORE_HASH="30ae3263c9b68d3bef06f799ba9d7a67e3fad447030625f0ffa4bb22684228b0"
TOXCORE_FILENAME="c-toxcore-$TOXCORE_VERSION.tar.gz"
if [ ! -f "$TOXCORE_PREFIX_DIR/done" ]
then
  rm -rf "$TOXCORE_PREFIX_DIR"
  mkdir -p "$TOXCORE_PREFIX_DIR"

  curl $CURL_OPTIONS https://github.com/TokTok/c-toxcore/archive/v$TOXCORE_VERSION.tar.gz -o $TOXCORE_FILENAME
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
        -DENABLE_STATIC=OFF \
        -DENABLE_SHARED=ON \
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


set +u
if [[ -n "$TRAVIS_CI_STAGE" ]] || [[ "$BUILD_TYPE" == "debug" ]]
then
set -u

  # mingw-w64-debug-scripts

  MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR="$DEP_DIR/mingw-w64-debug-scripts"
  MINGW_W64_DEBUG_SCRIPTS_VERSION="c6ae689137844d1a6fd9c1b9a071d3f82a44c593"
  MINGW_W64_DEBUG_SCRIPTS_HASH="1343bee72f3d9fad01ac7101d6e9cffee1e76db82f2ef9a69f7c7e988ec4b301"
  if [ ! -f "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/done" ]
  then
    rm -rf "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR"
    mkdir -p "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR"

    git clone https://github.com/nurupo/mingw-w64-debug-scripts mingw-w64-debug-scripts
    cd mingw-w64-debug-scripts
    git checkout $MINGW_W64_DEBUG_SCRIPTS_VERSION
    check_sha256_git "$MINGW_W64_DEBUG_SCRIPTS_HASH"

    sed -i "s|your-app-name.exe|qtox.exe|g" debug-*.bat
    mkdir -p "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/bin"
    cp -a debug-*.bat "$MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/bin/"
    echo -n $MINGW_W64_DEBUG_SCRIPTS_VERSION > $MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/done

    cd ..
    rm -rf ./mingw-w64-debug-scripts
  else
    echo "Using cached build of mingw-w64-debug-scripts `cat $MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/done`"
  fi


  # Expat

  EXPAT_PREFIX_DIR="$DEP_DIR/libexpat"
  EXPAT_VERSION="2.2.9"
  EXPAT_HASH="1ea6965b15c2106b6bbe883397271c80dfa0331cdf821b2c319591b55eadc0a4"
  EXPAT_FILENAME="expat-$EXPAT_VERSION.tar.xz"
  if [ ! -f "$EXPAT_PREFIX_DIR/done" ]
  then
    rm -rf "$EXPAT_PREFIX_DIR"
    mkdir -p "$EXPAT_PREFIX_DIR"

    curl $CURL_OPTIONS -O "https://github.com/libexpat/libexpat/releases/download/R_${EXPAT_VERSION//./_}/$EXPAT_FILENAME"
    check_sha256 "$EXPAT_HASH" "$EXPAT_FILENAME"
    bsdtar --no-same-owner --no-same-permissions -xf $EXPAT_FILENAME
    rm $EXPAT_FILENAME
    cd expat*

    CFLAGS="-O2 -g0" ./configure --host="$ARCH-w64-mingw32" \
                                 --prefix="$EXPAT_PREFIX_DIR" \
                                 --enable-static \
                                 --disable-shared
    make
    make install
    echo -n $EXPAT_VERSION > $EXPAT_PREFIX_DIR/done

    cd ..
    rm -rf ./expat*
  else
    echo "Using cached build of Expat `cat $EXPAT_PREFIX_DIR/done`"
  fi


  # GDB

  GDB_PREFIX_DIR="$DEP_DIR/gdb"
  GDB_VERSION="9.2"
  GDB_HASH="360cd7ae79b776988e89d8f9a01c985d0b1fa21c767a4295e5f88cb49175c555"
  GDB_FILENAME="gdb-$GDB_VERSION.tar.xz"
  if [ ! -f "$GDB_PREFIX_DIR/done" ]
  then
    rm -rf "$GDB_PREFIX_DIR"
    mkdir -p "$GDB_PREFIX_DIR"

    curl $CURL_OPTIONS -O "http://ftp.gnu.org/gnu/gdb/$GDB_FILENAME"
    check_sha256 "$GDB_HASH" "$GDB_FILENAME"
    bsdtar --no-same-owner --no-same-permissions -xf $GDB_FILENAME
    rm $GDB_FILENAME
    cd gdb*

    mkdir build
    cd build
    CFLAGS="-O2 -g0" ../configure --host="$ARCH-w64-mingw32" \
                                  --prefix="$GDB_PREFIX_DIR" \
                                  --enable-static \
                                  --disable-shared \
                                  --with-libexpat-prefix="$EXPAT_PREFIX_DIR"
    make
    make install
    cd ..
    echo -n $GDB_VERSION > $GDB_PREFIX_DIR/done

    cd ..
    rm -rf ./gdb*
  else
    echo "Using cached build of GDB `cat $GDB_PREFIX_DIR/done`"
  fi

set +u
fi
set -u


# NSIS ShellExecAsUser plugin

NSISSHELLEXECASUSER_PREFIX_DIR="$DEP_DIR/nsis_shellexecuteasuser"
NSISSHELLEXECASUSER_VERSION=" "
NSISSHELLEXECASUSER_HASH="8fc19829e144716a422b15a85e718e1816fe561de379b2b5ae87ef9017490799"
if [ ! -f "$NSISSHELLEXECASUSER_PREFIX_DIR/done" ]
then
  rm -rf "$NSISSHELLEXECASUSER_PREFIX_DIR"
  mkdir -p "$NSISSHELLEXECASUSER_PREFIX_DIR"

  # Backup: https://web.archive.org/web/20171008011417/http://nsis.sourceforge.net/mediawiki/images/c/c7/ShellExecAsUser.zip
  curl $CURL_OPTIONS -O http://nsis.sourceforge.net/mediawiki/images/c/c7/ShellExecAsUser.zip
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


# mingw-ldd

MINGW_LDD_PREFIX_DIR="$DEP_DIR/mingw-ldd"
MINGW_LDD_VERSION=v0.2.0
MINGW_LDD_HASH="d4cf712da18fa822b4934144d44cd254e18c9c0ca987363503bb3b6aeb3134db"
MINGW_LDD_FILENAME="$MINGW_LDD_VERSION.tar.gz"
if [ ! -f "$MINGW_LDD_PREFIX_DIR/done" ]
then
  rm -rf "$MINGW_LDD_PREFIX_DIR"
  mkdir -p "$MINGW_LDD_PREFIX_DIR"

  curl $CURL_OPTIONS "https://github.com/nurupo/mingw-ldd/archive/$MINGW_LDD_FILENAME" -o "$MINGW_LDD_FILENAME"
  check_sha256 "$MINGW_LDD_HASH" "$MINGW_LDD_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$MINGW_LDD_FILENAME"
  rm "$MINGW_LDD_FILENAME"
  cd mingw-ldd*

  mkdir "$MINGW_LDD_PREFIX_DIR/bin"
  cp -a "mingw_ldd/mingw_ldd.py" "$MINGW_LDD_PREFIX_DIR/bin/mingw-ldd.py"
  echo -n $MINGW_LDD_VERSION > $MINGW_LDD_PREFIX_DIR/done

  cd ..
  rm -rf ./mingw-ldd*
else
  echo "Using cached build of mingw-ldd `cat $MINGW_LDD_PREFIX_DIR/done`"
fi


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

# Run tests using Wine
set +u
if [[ -n "$TRAVIS_CI_STAGE" ]]
then
  echo "SET(TEST_CROSSCOMPILING_EMULATOR /usr/bin/wine)" >> toolchain.cmake
fi
set -u

# Spell check on windows currently not supported, disable
if [[ "$BUILD_TYPE" == "release" ]]
then
  cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DSPELL_CHECK=OFF \
        -DUPDATE_CHECK=ON \
        -DSTRICT_OPTIONS=ON \
        ..
elif [[ "$BUILD_TYPE" == "debug" ]]
then
  cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Debug \
        -DSPELL_CHECK=OFF \
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
cp {$OPENSSL_PREFIX_DIR,$SQLCIPHER_PREFIX_DIR,$FFMPEG_PREFIX_DIR,$OPENAL_PREFIX_DIR,$QRENCODE_PREFIX_DIR,$EXIF_PREFIX_DIR,$OPUS_PREFIX_DIR,$SODIUM_PREFIX_DIR,$VPX_PREFIX_DIR,$TOXCORE_PREFIX_DIR}/bin/*.dll $QTOX_PREFIX_DIR

cp /usr/lib/gcc/$ARCH-w64-mingw32/*-posix/libgcc_s_*.dll $QTOX_PREFIX_DIR
cp /usr/lib/gcc/$ARCH-w64-mingw32/*-posix/libstdc++-6.dll $QTOX_PREFIX_DIR
cp /usr/$ARCH-w64-mingw32/lib/libwinpthread-1.dll $QTOX_PREFIX_DIR

# Setup wine
if [[ "$ARCH" == "i686" ]]
then
  export WINEARCH=win32
elif [[ "$ARCH" == "x86_64" ]]
then
  export WINEARCH=win64
fi
winecfg

# dll checks
python3 $MINGW_LDD_PREFIX_DIR/bin/mingw-ldd.py $QTOX_PREFIX_DIR/qtox.exe --dll-lookup-dirs $QTOX_PREFIX_DIR ~/.wine/drive_c/windows/system32 > /tmp/$ARCH-qtox-ldd
find "$QTOX_PREFIX_DIR" -name '*.dll' > /tmp/$ARCH-qtox-dll-find
# dlls loded at run time that don't showup as a link time dependency
echo "$QTOX_PREFIX_DIR/libssl-1_1.dll
$QTOX_PREFIX_DIR/libssl-1_1-x64.dll
$QTOX_PREFIX_DIR/iconengines/qsvgicon.dll
$QTOX_PREFIX_DIR/imageformats/qgif.dll
$QTOX_PREFIX_DIR/imageformats/qico.dll
$QTOX_PREFIX_DIR/imageformats/qjpeg.dll
$QTOX_PREFIX_DIR/imageformats/qsvg.dll
$QTOX_PREFIX_DIR/platforms/qdirect2d.dll
$QTOX_PREFIX_DIR/platforms/qminimal.dll
$QTOX_PREFIX_DIR/platforms/qoffscreen.dll
$QTOX_PREFIX_DIR/platforms/qwindows.dll" > /tmp/$ARCH-qtox-dll-whitelist


# Check that all dlls are in place
if grep 'not found' /tmp/$ARCH-qtox-ldd
then
  cat /tmp/$ARCH-qtox-ldd
  echo "Error: Missing some dlls."
  exit 1
fi

# Check that no extra dlls get bundled
while IFS= read -r line
do
  # skip over whitelisted dlls
  if grep "$line" /tmp/$ARCH-qtox-dll-whitelist
  then
    continue
  fi
  if ! grep "$line" /tmp/$ARCH-qtox-ldd
  then
    echo "Error: extra dll included: $line. If this is a mistake and the dll is actually needed (e.g. it's loaded at run-time), please add it to the whitelist."
    exit 1
  fi
done < /tmp/$ARCH-qtox-dll-find


# Run tests (only on Travis)
set +u
if [[ -n "$TRAVIS_CI_STAGE" ]]
then
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

  # Get debug scripts
  cp -r $MINGW_W64_DEBUG_SCRIPTS_PREFIX_DIR/bin/* "$QTOX_PREFIX_DIR/"
  cp -r $GDB_PREFIX_DIR/bin/gdb.exe "$QTOX_PREFIX_DIR/"

  # Check that all dlls are in place
  python3 $MINGW_LDD_PREFIX_DIR/bin/mingw-ldd.py $QTOX_PREFIX_DIR/gdb.exe --dll-lookup-dirs $QTOX_PREFIX_DIR ~/.wine/drive_c/windows/system32 > /tmp/$ARCH-gdb-ldd
  if grep 'not found' /tmp/$ARCH-gdb-ldd
  then
    cat /tmp/$ARCH-gdb-ldd
    echo "Error: Missing some dlls."
    exit 1
  fi
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
