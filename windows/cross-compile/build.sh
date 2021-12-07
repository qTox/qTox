#!/usr/bin/env bash

# MIT License
#
# Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
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
if [ ! -f /etc/os-release ] || ! cat /etc/os-release | grep -qi 'bullseye'
then
  echo "Error: This script should be run on Debian Bullseye."
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
                   ca-certificates \
                   cmake \
                   extra-cmake-modules \
                   git \
                   libarchive-tools \
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
  update-alternatives --set i686-w64-mingw32-gcc /usr/bin/i686-w64-mingw32-gcc-posix
  update-alternatives --set i686-w64-mingw32-g++ /usr/bin/i686-w64-mingw32-g++-posix
elif [[ "$ARCH" == "x86_64" ]]
then
  apt-get install -y --no-install-recommends \
                    g++-mingw-w64-x86-64 \
                    gcc-mingw-w64-x86-64
  update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix
  update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix
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
OPENSSL_VERSION="1.1.1l"
# hash from https://www.openssl.org/source/
OPENSSL_HASH="0b7a3e5e59c34827fe0c3a74b7ec8baef302b98fa80088d7f9153aa16fa76bd1"
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
QT_PATCH=12
QT_VERSION=$QT_MAJOR.$QT_MINOR.$QT_PATCH
# hash from https://download.qt.io/archive/qt/5.12/5.12.12/single/qt-everywhere-src-5.12.12.tar.xz.mirrorlist
QT_HASH="1979a3233f689cb8b3e2783917f8f98f6a2e1821a70815fb737f020cd4b6ab06"
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
SQLCIPHER_VERSION=v4.5.0
SQLCIPHER_HASH="20c46a855c47d5a0a159fdcaa8491ec7bdbaa706a734ee52bc76188b929afb14"
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
FFMPEG_VERSION=4.4.1
FFMPEG_HASH="eadbad9e9ab30b25f5520fbfde99fae4a92a1ae3c0257a8d68569a4651e30e02"
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
QRENCODE_VERSION=4.1.1
QRENCODE_HASH="e455d9732f8041cf5b9c388e345a641fd15707860f928e94507b1961256a6923"
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
EXIF_VERSION="0.6.24"
EXIF_HASH="d47564c433b733d83b6704c70477e0a4067811d184ec565258ac563d8223f6ae"
EXIF_FILENAME="libexif-$EXIF_VERSION.tar.bz2"
if [ ! -f "$EXIF_PREFIX_DIR/done" ]
then
  rm -rf "$EXIF_PREFIX_DIR"
  mkdir -p "$EXIF_PREFIX_DIR"

  curl $CURL_OPTIONS -O "https://github.com/libexif/libexif/releases/download/v${EXIF_VERSION}/$EXIF_FILENAME"
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


# Snorenotify

SNORE_PREFIX_DIR="$DEP_DIR/snorenotify"
SNORE_VERSION=0.7.0
SNORE_HASH="2e3f5fbb80ab993f6149136cd9a14c2de66f48cabce550dead167a9448f5bed9"
SNORE_FILENAME="v$SNORE_VERSION.tar.gz"
if [ ! -f "$SNORE_PREFIX_DIR/done" ]
then
  rm -rf "$SNORE_PREFIX_DIR"
  mkdir -p "$SNORE_PREFIX_DIR"

  curl $CURL_OPTIONS -O "https://github.com/KDE/snorenotify/archive/${SNORE_FILENAME}"
  check_sha256 "$SNORE_HASH" "$SNORE_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf $SNORE_FILENAME
  rm $SNORE_FILENAME
  cd snorenotify*

  mkdir _build && cd _build

  export PKG_CONFIG_PATH="$QT_PREFIX_DIR/lib/pkgconfig"
  export PKG_CONFIG_LIBDIR="/usr/$ARCH-w64-mingw32"

  echo "
      SET(CMAKE_SYSTEM_NAME Windows)

      SET(CMAKE_C_COMPILER $ARCH-w64-mingw32-gcc)
      SET(CMAKE_CXX_COMPILER $ARCH-w64-mingw32-g++)
      SET(CMAKE_RC_COMPILER $ARCH-w64-mingw32-windres)

      SET(CMAKE_FIND_ROOT_PATH /usr/$ARCH-w64-mingw32 $QT_PREFIX_DIR)
  " > toolchain.cmake

  cmake -DCMAKE_INSTALL_PREFIX="$SNORE_PREFIX_DIR" \
        -DCMAKE_BUILD_TYPE=Relase \
        -DBUILD_daemon=OFF \
        -DBUILD_settings=OFF \
        -DBUILD_snoresend=OFF \
        -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake \
        ..

  make
  make install
  echo -n $SNORE_VERSION > $SNORE_PREFIX_DIR/done

  unset PKG_CONFIG_PATH
  unset PKG_CONFIG_LIBDIR

  cd ..

  cd ..
  rm -rf ./snorenotify*
else
  echo "Using cached build of snorenotify `cat $SNORE_PREFIX_DIR/done`"
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

  LDFLAGS="-fstack-protector" \
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

  LDFLAGS="-fstack-protector" \
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
VPX_VERSION=v1.10.0
VPX_HASH="85803ccbdbdd7a3b03d930187cb055f1353596969c1f92ebec2db839fa4f834a"
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


# ToxExt

TOXEXT_PREFIX_DIR="$DEP_DIR/toxext"
TOXEXT_VERSION=0.0.3
TOXEXT_HASH="99cf215d261a07bd83eafd1c69dcf78018db605898350b6137f1fd8e7c54734a"
TOXEXT_FILENAME="toxext-$TOXEXT_VERSION.tar.gz"
if [ ! -f "$TOXEXT_PREFIX_DIR/done" ]
then
  rm -rf "$TOXEXT_PREFIX_DIR"
  mkdir -p "$TOXEXT_PREFIX_DIR"

  curl $CURL_OPTIONS https://github.com/toxext/toxext/archive/v$TOXEXT_VERSION.tar.gz -o $TOXEXT_FILENAME
  check_sha256 "$TOXEXT_HASH" "$TOXEXT_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$TOXEXT_FILENAME"
  rm "$TOXEXT_FILENAME"
  cd toxext*

  mkdir -p build
  cd build

  export PKG_CONFIG_PATH="$OPUS_PREFIX_DIR/lib/pkgconfig:$SODIUM_PREFIX_DIR/lib/pkgconfig:$VPX_PREFIX_DIR/lib/pkgconfig:$TOXCORE_PREFIX_DIR/lib/pkgconfig"
  export PKG_CONFIG_LIBDIR="/usr/$ARCH-w64-mingw32"

  echo "
      SET(CMAKE_SYSTEM_NAME Windows)

      SET(CMAKE_C_COMPILER $ARCH-w64-mingw32-gcc)
      SET(CMAKE_CXX_COMPILER $ARCH-w64-mingw32-g++)
      SET(CMAKE_RC_COMPILER $ARCH-w64-mingw32-windres)

      SET(CMAKE_FIND_ROOT_PATH /usr/$ARCH-w64-mingw32 $OPUS_PREFIX_DIR $SODIUM_PREFIX_DIR $VPX_PREFIX_DIR $TOXCORE_PREFIX_DIR)
  " > toolchain.cmake

  cmake -DCMAKE_INSTALL_PREFIX=$TOXEXT_PREFIX_DIR \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
        ..

  make
  make install
  echo -n $TOXEXT_VERSION > $TOXEXT_PREFIX_DIR/done

  unset PKG_CONFIG_PATH
  unset PKG_CONFIG_LIBDIR

  cd ..

  cd ..
  rm -rf ./toxext*
else
  echo "Using cached build of ToxExt `cat $TOXEXT_PREFIX_DIR/done`"
fi


# tox_extension_messages

TOX_EXTENSION_MESSAGES_PREFIX_DIR="$DEP_DIR/tox_extension_messages"
TOX_EXTENSION_MESSAGES_VERSION=0.0.3
TOX_EXTENSION_MESSAGES_HASH="e7a9a199a3257382a85a8e555b6c8c540b652a11ca9a471b9da2a25a660dfdc3"
TOX_EXTENSION_MESSAGES_FILENAME="tox_extension_messages-$TOX_EXTENSION_MESSAGES_VERSION.tar.gz"
if [ ! -f "$TOX_EXTENSION_MESSAGES_PREFIX_DIR/done" ]
then
  rm -rf "$TOX_EXTENSION_MESSAGES_PREFIX_DIR"
  mkdir -p "$TOX_EXTENSION_MESSAGES_PREFIX_DIR"

  curl $CURL_OPTIONS https://github.com/toxext/tox_extension_messages/archive/v$TOX_EXTENSION_MESSAGES_VERSION.tar.gz -o $TOX_EXTENSION_MESSAGES_FILENAME
  check_sha256 "$TOX_EXTENSION_MESSAGES_HASH" "$TOX_EXTENSION_MESSAGES_FILENAME"
  bsdtar --no-same-owner --no-same-permissions -xf "$TOX_EXTENSION_MESSAGES_FILENAME"
  rm "$TOX_EXTENSION_MESSAGES_FILENAME"
  cd tox_extension_messages*

  mkdir -p build
  cd build

  export PKG_CONFIG_PATH="$OPUS_PREFIX_DIR/lib/pkgconfig:$SODIUM_PREFIX_DIR/lib/pkgconfig:$VPX_PREFIX_DIR/lib/pkgconfig:$TOXCORE_PREFIX_DIR/lib/pkgconfig"
  export PKG_CONFIG_LIBDIR="/usr/$ARCH-w64-mingw32"

  echo "
      SET(CMAKE_SYSTEM_NAME Windows)

      SET(CMAKE_C_COMPILER $ARCH-w64-mingw32-gcc)
      SET(CMAKE_CXX_COMPILER $ARCH-w64-mingw32-g++)
      SET(CMAKE_RC_COMPILER $ARCH-w64-mingw32-windres)

      SET(CMAKE_FIND_ROOT_PATH /usr/$ARCH-w64-mingw32 $OPUS_PREFIX_DIR $SODIUM_PREFIX_DIR $VPX_PREFIX_DIR $TOXCORE_PREFIX_DIR $TOXEXT_PREFIX_DIR)
  " > toolchain.cmake

  cmake -DCMAKE_INSTALL_PREFIX=$TOX_EXTENSION_MESSAGES_PREFIX_DIR \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
        ..

  make
  make install
  echo -n $TOX_EXTENSION_MESSAGES_VERSION > $TOX_EXTENSION_MESSAGES_PREFIX_DIR/done

  unset PKG_CONFIG_PATH
  unset PKG_CONFIG_LIBDIR

  cd ..

  cd ..
  rm -rf ./tox_extension_messages*
else
  echo "Using cached build of tox_extension_messages `cat $TOX_EXTENSION_MESSAGES_PREFIX_DIR/done`"
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
  EXPAT_VERSION="2.4.1"
  EXPAT_HASH="cf032d0dba9b928636548e32b327a2d66b1aab63c4f4a13dd132c2d1d2f2fb6a"
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


  # GMP

  GMP_PREFIX_DIR="$DEP_DIR/libgmp"
  GMP_VERSION="6.2.1"
  GMP_HASH="fd4829912cddd12f84181c3451cc752be224643e87fac497b69edddadc49b4f2"
  GMP_FILENAME="gmp-$GMP_VERSION.tar.xz"
  if [ ! -f "$GMP_PREFIX_DIR/done" ]
  then
    rm -rf "$GMP_PREFIX_DIR"
    mkdir -p "$GMP_PREFIX_DIR"

    curl $CURL_OPTIONS -O "http://ftp.gnu.org/gnu/gmp/$GMP_FILENAME"
    check_sha256 "$GMP_HASH" "$GMP_FILENAME"
    bsdtar --no-same-owner --no-same-permissions -xf $GMP_FILENAME
    rm $GMP_FILENAME
    cd gmp*

    mkdir build
    cd build
    CFLAGS="-O2 -g0" ../configure --host="$ARCH-w64-mingw32" \
                                  --prefix="$GMP_PREFIX_DIR" \
                                  --enable-static \
                                  --disable-shared
    make
    make install
    cd ..
    echo -n $GMP_VERSION > $GMP_PREFIX_DIR/done

    cd ..
    rm -rf ./gmp*
  else
    echo "Using cached build of GMP `cat $GMP_PREFIX_DIR/done`"
  fi


  # GDB

  GDB_PREFIX_DIR="$DEP_DIR/gdb"
  GDB_VERSION="11.1"
  GDB_HASH="cccfcc407b20d343fb320d4a9a2110776dd3165118ffd41f4b1b162340333f94"
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
                                  --with-libexpat-prefix="$EXPAT_PREFIX_DIR" \
                                  --with-libgmp-prefix="$GMP_PREFIX_DIR"
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
MINGW_LDD_VERSION=v0.2.1
MINGW_LDD_HASH="60d34506d2f345e011b88de172ef312f37ca3ba87f3764f511061b69878ab204"
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
        -DDESKTOP_NOTIFICATIONS=ON \
        -DUPDATE_CHECK=ON \
        -DSTRICT_OPTIONS=ON \
        ..
elif [[ "$BUILD_TYPE" == "debug" ]]
then
  cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Debug \
        -DSPELL_CHECK=OFF \
        -DDESKTOP_NOTIFICATIONS=ON \
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

cp "$SNORE_PREFIX_DIR/bin/libsnore-qt5.dll" $QTOX_PREFIX_DIR
mkdir -p "$QTOX_PREFIX_DIR/libsnore-qt5"
cp "$SNORE_PREFIX_DIR/lib/plugins/libsnore-qt5/libsnore_backend_windowstoast.dll" "$QTOX_PREFIX_DIR/libsnore-qt5"
cp "$SNORE_PREFIX_DIR/bin/SnoreToast.exe" $QTOX_PREFIX_DIR

cp /usr/lib/gcc/$ARCH-w64-mingw32/*-posix/{libgcc_s_*.dll,libstdc++*.dll,libssp*.dll} $QTOX_PREFIX_DIR
cp /usr/$ARCH-w64-mingw32/lib/libwinpthread*.dll $QTOX_PREFIX_DIR

# Setup wine
# Note that SQLCipher and FFmpeg (maybe more?) seem to setup ~/.wine on their
# own, but to the wrong bitness (always 64-bit?), when we want a matching
# bitness here for mingw-ldd, so remove it before proceeding.
rm -rf ~/.wine
if [[ "$ARCH" == "i686" ]]
then
  export WINEARCH=win32
elif [[ "$ARCH" == "x86_64" ]]
then
  export WINEARCH=win64
fi
winecfg

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

# dll check
# Create lists of all .exe and .dll files
find "$QTOX_PREFIX_DIR" -iname '*.dll' > dlls
find "$QTOX_PREFIX_DIR" -iname '*.exe' > exes

# Create a list of dlls that are loaded during the runtime (not listed in the PE
# import table, thus ldd doesn't print those)
echo "$QTOX_PREFIX_DIR/libsnore-qt5/libsnore_backend_windowstoast.dll
$QTOX_PREFIX_DIR/iconengines/qsvgicon.dll
$QTOX_PREFIX_DIR/imageformats/qgif.dll
$QTOX_PREFIX_DIR/imageformats/qico.dll
$QTOX_PREFIX_DIR/imageformats/qjpeg.dll
$QTOX_PREFIX_DIR/imageformats/qsvg.dll
$QTOX_PREFIX_DIR/platforms/qdirect2d.dll
$QTOX_PREFIX_DIR/platforms/qminimal.dll
$QTOX_PREFIX_DIR/platforms/qoffscreen.dll
$QTOX_PREFIX_DIR/platforms/qwindows.dll" > runtime-dlls
if [[ "$ARCH" == "i686" ]]
then
  echo "$QTOX_PREFIX_DIR/libssl-1_1.dll" >> runtime-dlls
elif [[ "$ARCH" == "x86_64" ]]
then
  echo "$QTOX_PREFIX_DIR/libssl-1_1-x64.dll" >> runtime-dlls
fi

# Create a tree of all required dlls
# Assumes all .exe files are directly in $QTOX_PREFIX_DIR, not in subdirs
while IFS= read -r line
do
  if [[ "$ARCH" == "i686" ]]
  then
    WINE_DLL_DIR="/root/.wine/drive_c/windows/system32"
  elif [[ "$ARCH" == "x86_64" ]]
  then
    WINE_DLL_DIR="/root/.wine/drive_c/windows/system32 /root/.wine/drive_c/windows/syswow64"
  fi
  python3 $MINGW_LDD_PREFIX_DIR/bin/mingw-ldd.py $line --dll-lookup-dirs $QTOX_PREFIX_DIR $WINE_DLL_DIR --output-format tree >> dlls-required
done < <(cat exes runtime-dlls)

# Check that no extra dlls get bundled
while IFS= read -r line
do
  if ! grep -q "$line" dlls-required
  then
    echo "Error: extra dll included: $line. If this is a mistake and the dll is actually needed (e.g. it's loaded at runtime), please add it to the runtime dll list."
    exit 1
  fi
done < dlls

# Check that no dll is missing
if grep -q 'not found' dlls-required
then
  cat dlls-required
  echo "Error: Missing some dlls."
  exit 1
fi

cd ..
rm -rf ./qtox

# Cache APT packages for future runs
store_apt_cache

# Chmod since everything is root:root
chmod 777 -R "$WORKSPACE_DIR"
