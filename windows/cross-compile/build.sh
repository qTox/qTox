#!/usr/bin/env bash

# MIT License
#
# Copyright (c) 2017 Maxim Biro <nurupo.contributions@gmail.com>
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
#   because it requires static Qt, which means we'd need to build Qt twice,
#   and building Qt takes really long time.
#
# - Doesn't create an installer because there is no NSIS 3 in Ubuntu. We could
#   switch to Debian instead and backport it from Experimental, which is what
#   we do on Jenkins, but since we don't build an updater, we might as well
#   just do the nightly qTox build: no updater, no installer.
#
# - Qt from 5.8.0 to 5.9.1 doesn't cross-compile to Windows due to
#   https://bugreports.qt.io/browse/QTBUG-61740. Should be fixed in 5.9.2. You
#   can easily patch it though, just one sed command, but I guess we will wait
#   for 5.9.2.
#
# - FFmpeg 3.3 doesn't cross-compile correctly, qTox build fails when linking
#   against the 3.3 FFmpeg. They have removed `--enable-memalign-hack` switch,
#   which might be what causes this.


set -euo pipefail


# Common directory paths

readonly WORKSPACE_DIR="/workspace"
readonly SCRIPT_DIR="/script"
readonly QTOX_SRC_DIR="/qtox"


# Make sure we run in an expected environment

if ! grep -q 'buntu 16\.04' /etc/lsb-release
then
  echo "Error: This script should be run on Ubuntu 16.04."
  exit 1
fi

if [ ! -d "$WORKSPACE_DIR" ] || [ ! -d "$SCRIPT_DIR" ] || [ ! -d "$QTOX_SRC_DIR" ]
then
  echo "Error: At least one of $WORKSPACE_DIR, $SCRIPT_DIR or $QTOX_SRC_DIR directories is missing."
  exit 1
fi

if [ ! -d "$QTOX_SRC_DIR/src" ]
then
  echo "Error: $QTOX_SRC_DIR/src directory is missing, make sure $QTOX_SRC_DIR contains qTox source code."
  exit 1
fi

if [ "$(id -u)" != "0" ]
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


set -x


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
                   pkg-config \
                   tclsh \
                   unzip \
                   wget \
                   yasm
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


# Create the expected directory structure

# Just make sure those exist
mkdir -p "$WORKSPACE_DIR"
mkdir -p "$DEP_DIR"

# Build dir should be empty
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"


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


# OpenSSL

OPENSSL_PREFIX_DIR="$DEP_DIR/libopenssl"
OPENSSL_VERSION=1.0.2l
if [ ! -f "$OPENSSL_PREFIX_DIR/done" ]
then
  rm -rf "$OPENSSL_PREFIX_DIR"
  mkdir -p "$OPENSSL_PREFIX_DIR"

  wget https://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz
  check_sha256 "ce07195b659e75f4e1db43552860070061f156a98bb37b672b101ba6e3ddf30c" "openssl-$OPENSSL_VERSION.tar.gz"
  bsdtar -xf openssl*.tar.gz
  rm openssl*.tar.gz
  cd openssl*

  CONFIGURE_OPTIONS="--prefix=$OPENSSL_PREFIX_DIR shared"
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
QT_MINOR=6
QT_PATCH=3
QT_VERSION=$QT_MAJOR.$QT_MINOR.$QT_PATCH
if [ ! -f "$QT_PREFIX_DIR/done" ]
then
  rm -rf "$QT_PREFIX_DIR"
  mkdir -p "$QT_PREFIX_DIR"

  QT_MIRROR=http://qt.mirror.constant.com

  wget $QT_MIRROR/official_releases/qt/$QT_MAJOR.$QT_MINOR/$QT_VERSION/single/qt-everywhere-opensource-src-$QT_VERSION.tar.xz
  check_sha256 "2fa0cf2e5e8841b29a4be62062c1a65c4f6f2cf1beaf61a5fd661f520cd776d0" "qt-everywhere-opensource-src-$QT_VERSION.tar.xz"
  bsdtar -xf qt*.tar.xz
  rm qt*.tar.xz
  cd qt*

  export PKG_CONFIG_PATH="$OPENSSL_PREFIX_DIR/lib/pkgconfig"
  export OPENSSL_LIBS="$(pkg-config --libs openssl)"

  # So, apparently Travis CI terminate a build if it generates more than 4mb of output
  # which happens when building Qt
  CONFIGURE_EXTRA=""
  set +u
  if [ "$TRAVIS_CI_STAGE_ONE" == "true" ]
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
    -skip translations \
    -skip doc \
    -skip qtdeclarative \
    -skip activeqt \
    -skip qtwebsockets \
    -skip qtscript \
    -skip qtquickcontrols \
    -skip qtconnectivity \
    -skip qtenginio \
    -skip qtsensors \
    -skip qtserialport \
    -skip qtlocation \
    -skip qtgraphicaleffects \
    -skip qtimageformats \
    -skip webchannel \
    -skip multimedia \
    -no-dbus \
    -no-icu \
    -no-qml-debug \
    -no-compile-examples \
    -qt-libjpeg \
    -qt-libpng \
    -qt-zlib \
    -qt-pcre $CONFIGURE_EXTRA

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
if [ "$TRAVIS_CI_STAGE_ONE" == "true" ]
then
  # Strip to reduce cache size
  strip_all
  exit 1
fi
set -u


# SQLCipher

SQLCIPHER_PREFIX_DIR="$DEP_DIR/libsqlcipher"
SQLCIPHER_VERSION=v3.4.1
if [ ! -f "$SQLCIPHER_PREFIX_DIR/done" ]
then
  rm -rf "$SQLCIPHER_PREFIX_DIR"
  mkdir -p "$SQLCIPHER_PREFIX_DIR"

  git clone \
    --branch $SQLCIPHER_VERSION \
    --depth 1 \
    https://github.com/sqlcipher/sqlcipher \
    sqlcipher
  cd sqlcipher

  sed -i s/'LIBS="-lcrypto  $LIBS"'/'LIBS="-lcrypto -lgdi32  $LIBS"'/g configure
  sed -i s/'LIBS="-lcrypto $LIBS"'/'LIBS="-lcrypto -lgdi32  $LIBS"'/g configure
  sed -i s/'if test "$TARGET_EXEEXT" = ".exe"'/'if test ".exe" = ".exe"'/g configure

# Do not remove trailing whitespace and dont replace tabs with spaces in the patch below,
#  otherwise the patch will fail to apply
> Makefile.in-patch cat << "EOF"
--- Makefile.in	2017-07-24 04:33:46.944080013 +0000
+++ Makefile.in-patch	2017-07-24 04:50:47.340596990 +0000
@@ -1074,7 +1074,7 @@
    $(TOP)/ext/fts5/fts5_varint.c \
    $(TOP)/ext/fts5/fts5_vocab.c  \
 
-fts5parse.c:	$(TOP)/ext/fts5/fts5parse.y lemon 
+fts5parse.c:	$(TOP)/ext/fts5/fts5parse.y lemon$(BEXE)
 	cp $(TOP)/ext/fts5/fts5parse.y .
 	rm -f fts5parse.h
 	./lemon$(BEXE) $(OPTS) fts5parse.y

EOF

  patch -l < Makefile.in-patch

  ./configure --host="$ARCH-w64-mingw32" \
              --prefix="$SQLCIPHER_PREFIX_DIR" \
              --disable-shared \
              --enable-tempstore=yes \
              CFLAGS="-O2 -g0 -DSQLITE_HAS_CODEC -I$OPENSSL_PREFIX_DIR/include/" \
              LDFLAGS="$OPENSSL_PREFIX_DIR/lib/libcrypto.a -lcrypto -lgdi32 -L$OPENSSL_PREFIX_DIR/lib/"

  sed -i s/"TEXE = $"/"TEXE = .exe"/ Makefile

  make
  make install
  echo -n $SQLCIPHER_VERSION > $SQLCIPHER_PREFIX_DIR/done

  cd ..
  rm -rf ./sqlcipher
else
  echo "Using cached build of SQLCipher `cat $SQLCIPHER_PREFIX_DIR/done`"
fi


# FFmpeg

FFMPEG_PREFIX_DIR="$DEP_DIR/libffmpeg"
FFMPEG_VERSION=3.2.8
if [ ! -f "$FFMPEG_PREFIX_DIR/done" ]
then
  rm -rf "$FFMPEG_PREFIX_DIR"
  mkdir -p "$FFMPEG_PREFIX_DIR"

  wget https://www.ffmpeg.org/releases/ffmpeg-$FFMPEG_VERSION.tar.xz
  check_sha256 "42e7362692318afc666f14378dd445effa9a1b09787504a6ab5811fe442674cd" "ffmpeg-$FFMPEG_VERSION.tar.xz"
  bsdtar -xf ffmpeg*.tar.xz
  cd ffmpeg*

  if [[ "$ARCH" == "x86_64"* ]]
  then
    CONFIGURE_OPTIONS="--arch=x86_64"
  elif [[ "$ARCH" == "i686" ]]
  then
    CONFIGURE_OPTIONS="--arch=x86"
  fi

  ./configure $CONFIGURE_OPTIONS \
              --prefix="$FFMPEG_PREFIX_DIR" \
              --target-os="mingw32" \
              --cross-prefix="$ARCH-w64-mingw32-" \
              --pkg-config="pkg-config" \
              --extra-cflags="-static -O2 -g0" \
              --extra-ldflags="-lm -static" \
              --pkg-config-flags="--static" \
              --disable-shared \
              --disable-programs \
              --disable-protocols \
              --disable-doc \
              --disable-sdl \
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
              --enable-demuxer=h264 \
              --enable-demuxer=mjpeg \
              --enable-parser=h264 \
              --enable-parser=mjpeg \
              --enable-decoder=h264 \
              --enable-decoder=mjpeg \
              --enable-memalign-hack
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

OPENAL_PREFIX_DIR="$DEP_DIR/libopenal"
OPENAL_VERSION=b80570bed017de60b67c6452264c634085c3b148
if [ ! -f "$OPENAL_PREFIX_DIR/done" ]
then
  rm -rf "$OPENAL_PREFIX_DIR"
  mkdir -p "$OPENAL_PREFIX_DIR"

  git clone https://github.com/irungentoo/openal-soft-tox openal-soft-tox
  cd openal*
  git checkout $OPENAL_VERSION

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
    -DDSOUND_LIBRARY=/usr/$ARCH-w64-mingw32/lib/libdsound.a

  make
  make install
  echo -n $OPENAL_VERSION > $OPENAL_PREFIX_DIR/done

  cd ..
  rm -rf ./openal*
else
  echo "Using cached build of irungentoo's OpenAL-Soft fork `cat $OPENAL_PREFIX_DIR/done`"
fi


# Filteraudio

FILTERAUDIO_PREFIX_DIR="$DEP_DIR/libfilteraudio"
FILTERAUDIO_VERSION=ada2f4fdc04940cdeee47caffe43add4fa017096
if [ ! -f "$FILTERAUDIO_PREFIX_DIR/done" ]
then
  rm -rf "$FILTERAUDIO_PREFIX_DIR"
  mkdir -p "$FILTERAUDIO_PREFIX_DIR"

  git clone https://github.com/irungentoo/filter_audio filter_audio
  cd filter*
  git checkout $FILTERAUDIO_VERSION

  $ARCH-w64-mingw32-gcc -O2 -g0 -c \
               aec/aec_core.c \
               aec/aec_core_sse2.c \
               aec/aec_rdft.c \
               aec/aec_rdft_sse2.c \
               aec/aec_resampler.c \
               aec/echo_cancellation.c \
               agc/analog_agc.c \
               agc/digital_agc.c \
               ns/ns_core.c \
               ns/nsx_core_c.c \
               ns/nsx_core.c \
               ns/noise_suppression_x.c \
               ns/noise_suppression.c \
               other/get_scaling_square.c \
               other/resample_by_2.c \
               other/spl_sqrt.c \
               other/delay_estimator.c \
               other/complex_bit_reverse.c \
               other/dot_product_with_scale.c \
               other/cross_correlation.c \
               other/min_max_operations.c \
               other/resample_48khz.c \
               other/high_pass_filter.c \
               other/energy.c \
               other/randomization_functions.c \
               other/speex_resampler.c \
               other/copy_set_operations.c \
               other/downsample_fast.c \
               other/complex_fft.c \
               other/vector_scaling_operations.c \
               other/resample_by_2_internal.c \
               other/delay_estimator_wrapper.c \
               other/real_fft.c \
               other/spl_sqrt_floor.c \
               other/resample_fractional.c \
               other/ring_buffer.c \
               other/splitting_filter.c \
               other/fft4g.c \
               other/division_operations.c \
               other/spl_init.c \
               other/float_util.c \
               zam/filters.c \
               vad/vad_sp.c \
               vad/vad_core.c \
               vad/webrtc_vad.c \
               vad/vad_gmm.c \
               vad/vad_filterbank.c \
               filter_audio.c \
               -lpthread \
               -lm

  ar rcs libfilteraudio.a \
    aec_core.o \
    aec_core_sse2.o \
    aec_rdft.o \
    aec_rdft_sse2.o \
    aec_resampler.o \
    echo_cancellation.o \
    analog_agc.o \
    digital_agc.o \
    ns_core.o \
    nsx_core_c.o \
    nsx_core.o \
    noise_suppression_x.o \
    noise_suppression.o \
    get_scaling_square.o \
    resample_by_2.o \
    spl_sqrt.o \
    delay_estimator.o \
    complex_bit_reverse.o \
    dot_product_with_scale.o \
    cross_correlation.o \
    min_max_operations.o \
    resample_48khz.o \
    high_pass_filter.o \
    energy.o \
    randomization_functions.o \
    speex_resampler.o \
    copy_set_operations.o \
    downsample_fast.o \
    complex_fft.o \
    vector_scaling_operations.o \
    resample_by_2_internal.o \
    delay_estimator_wrapper.o \
    real_fft.o \
    spl_sqrt_floor.o \
    resample_fractional.o \
    ring_buffer.o \
    splitting_filter.o \
    fft4g.o \
    division_operations.o \
    spl_init.o \
    float_util.o \
    filters.o \
    vad_sp.o \
    vad_core.o \
    webrtc_vad.o \
    vad_gmm.o \
    vad_filterbank.o \
    filter_audio.o

  mkdir $FILTERAUDIO_PREFIX_DIR/include
  mkdir $FILTERAUDIO_PREFIX_DIR/lib
  cp filter_audio.h $FILTERAUDIO_PREFIX_DIR/include
  cp libfilteraudio.a $FILTERAUDIO_PREFIX_DIR/lib
  echo -n $FILTERAUDIO_VERSION > $FILTERAUDIO_PREFIX_DIR/done

  cd ..
  rm -rf ./filter*
else
  echo "Using cached build of Filteraudio `cat $FILTERAUDIO_PREFIX_DIR/done`"
fi


# QREncode

QRENCODE_PREFIX_DIR="$DEP_DIR/libqrencode"
QRENCODE_VERSION=3.4.4
if [ ! -f "$QRENCODE_PREFIX_DIR/done" ]
then
  rm -rf "$QRENCODE_PREFIX_DIR"
  mkdir -p "$QRENCODE_PREFIX_DIR"

  wget https://fukuchi.org/works/qrencode/qrencode-$QRENCODE_VERSION.tar.bz2
  check_sha256 "efe5188b1ddbcbf98763b819b146be6a90481aac30cfc8d858ab78a19cde1fa5" "qrencode-$QRENCODE_VERSION.tar.bz2"
  bsdtar -xf qrencode*.tar.bz2
  rm qrencode*.tar.bz2
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
EXIF_VERSION=0.6.21
if [ ! -f "$EXIF_PREFIX_DIR/done" ]
then
  rm -rf "$EXIF_PREFIX_DIR"
  mkdir -p "$EXIF_PREFIX_DIR"

  wget https://sourceforge.net/projects/libexif/files/libexif/0.6.21/libexif-$EXIF_VERSION.tar.bz2
  check_sha256 "16cdaeb62eb3e6dfab2435f7d7bccd2f37438d21c5218ec4e58efa9157d4d41a" "libexif-$EXIF_VERSION.tar.bz2"
  bsdtar -xf libexif*.tar.bz2
  rm libexif*.tar.bz2
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


# Opus

OPUS_PREFIX_DIR="$DEP_DIR/libopus"
OPUS_VERSION=v1.2.1
if [ ! -f "$OPUS_PREFIX_DIR/done" ]
then
  rm -rf "$OPUS_PREFIX_DIR"
  mkdir -p "$OPUS_PREFIX_DIR"

  git clone \
    --branch $OPUS_VERSION \
    --depth 1 \
    git://git.opus-codec.org/opus.git \
    opus
  cd opus

  ./autogen.sh
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
  rm -rf ./opus
else
  echo "Using cached build of Opus `cat $OPUS_PREFIX_DIR/done`"
fi


# Sodium

SODIUM_PREFIX_DIR="$DEP_DIR/libsodium"
SODIUM_VERSION=1.0.15
if [ ! -f "$SODIUM_PREFIX_DIR/done" ]
then
  rm -rf "$SODIUM_PREFIX_DIR"
  mkdir -p "$SODIUM_PREFIX_DIR"

  git clone \
    --branch $SODIUM_VERSION \
    --depth 1 \
    https://github.com/jedisct1/libsodium \
    libsodium
  cd libsodium

  ./autogen.sh
  ./configure --host="$ARCH-w64-mingw32" \
              --prefix="$SODIUM_PREFIX_DIR" \
              --disable-shared \
              --enable-static \
              --with-pic
  make
  make install
  echo -n $SODIUM_VERSION > $SODIUM_PREFIX_DIR/done

  cd ..
  rm -rf ./libsodium
else
  echo "Using cached build of Sodium `cat $SODIUM_PREFIX_DIR/done`"
fi


# VPX

VPX_PREFIX_DIR="$DEP_DIR/libvpx"
VPX_VERSION=v1.6.1
if [ ! -f "$VPX_PREFIX_DIR/done" ]
then
  rm -rf "$VPX_PREFIX_DIR"
  mkdir -p "$VPX_PREFIX_DIR"

  git clone \
    --branch $VPX_VERSION \
    --depth 1 \
    https://github.com/webmproject/libvpx \
    libvpx
  cd libvpx

  if [[ "$ARCH" == "x86_64" ]]
  then
    VPX_TARGET=x86_64-win64-gcc
  elif [[ "$ARCH" == "i686" ]]
  then
    VPX_TARGET=x86-win32-gcc
  fi

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
  rm -rf ./libvpx
else
  echo "Using cached build of VPX `cat $VPX_PREFIX_DIR/done`"
fi


# Toxcore

TOXCORE_PREFIX_DIR="$DEP_DIR/libtoxcore"
TOXCORE_VERSION=v0.1.10
if [ ! -f "$TOXCORE_PREFIX_DIR/done" ]
then
  rm -rf "$TOXCORE_PREFIX_DIR"
  mkdir -p "$TOXCORE_PREFIX_DIR"

  git clone \
    --branch $TOXCORE_VERSION \
    --depth 1 \
    https://github.com/TokTok/c-toxcore \
    c-toxcore
  cd c-toxcore

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
               -DWARNINGS=OFF \
               -DCMAKE_BUILD_TYPE=Release \
               -DENABLE_STATIC=ON \
               -DENABLE_SHARED=OFF \
               -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake

  make
  make install
  echo -n $TOXCORE_VERSION > $TOXCORE_PREFIX_DIR/done

  unset PKG_CONFIG_PATH
  unset PKG_CONFIG_LIBDIR

  cd ..
  rm -rf ./c-toxcore
else
  echo "Using cached build of Toxcore `cat $TOXCORE_PREFIX_DIR/done`"
fi


# Stop here if running the second stage on Travis CI
set +u
if [ "$TRAVIS_CI_STAGE_TWO" == "true" ]
then
  # Strip to reduce cache size
  strip_all
  exit 1
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
mkdir build
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

if [[ "$BUILD_TYPE" == "release" ]]
then
  cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        ..
elif [[ "$BUILD_TYPE" == "debug" ]]
then
  cmake -DCMAKE_TOOLCHAIN_FILE=./toolchain.cmake \
        -DCMAKE_BUILD_TYPE=Debug \
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
cp $OPENSSL_PREFIX_DIR/bin/ssleay32.dll \
   $OPENSSL_PREFIX_DIR/bin/libeay32.dll \
   $QTOX_PREFIX_DIR
cp /usr/lib/gcc/$ARCH-w64-mingw32/*-posix/libgcc_s_*.dll $QTOX_PREFIX_DIR
cp /usr/lib/gcc/$ARCH-w64-mingw32/*-posix/libstdc++-6.dll $QTOX_PREFIX_DIR
cp /usr/$ARCH-w64-mingw32/lib/libwinpthread-1.dll $QTOX_PREFIX_DIR

cd ..

# Setup gdb
if [[ "$BUILD_TYPE" == "debug" ]]
then
  # Copy over qTox source code so that dbg could pick it up
  mkdir -p "$QTOX_PREFIX_DIR/$PWD/src"
  cp -r "$PWD/src" "$QTOX_PREFIX_DIR/$PWD"

  # Get dbg executable and the debug scripts
  git clone https://github.com/nurupo/mingw-w64-debug-scripts
  cd mingw-w64-debug-scripts
  git checkout 7341e1ffdea352e5557f3fcae51569f13e1ef270
  make $ARCH EXE_NAME=qtox.exe
  mv output/$ARCH/* "$QTOX_PREFIX_DIR/"
  cd ..
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

cd ..
rm -rf ./qtox

# Chmod since everything is root:root
chmod 777 -R "$WORKSPACE_DIR"
