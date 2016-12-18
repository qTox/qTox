#!/bin/bash
#
#    Copyright © 2015-2016 by The qTox Project Contributors
#
#    This program is libre software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# stop as soon as one of steps will fail
set -e -o pipefail

# Qt 5.3, since that's the lowest supported version
sudo add-apt-repository -y ppa:beineri/opt-qt532-trusty
sudo apt-get update -qq

# install needed Qt, OpenAL, opus, qrencode, GTK tray deps, sqlcipher
# `--force-yes` since we don't care about GPG failing to work with short IDs
sudo apt-get install -y --force-yes \
    automake \
    autotools-dev \
    build-essential \
    check \
    checkinstall \
    libgdk-pixbuf2.0-dev \
    libglib2.0-dev \
    libgtk2.0-dev \
    libopenal-dev \
    libopus-dev \
    libqrencode-dev \
    libsqlcipher-dev \
    libtool \
    libvpx-dev \
    libxss-dev qrencode \
    qt53base \
    qt53script \
    qt53svg \
    qt53tools \
    qt53xmlpatterns \
    pkg-config || yes

# Qt
source /opt/qt53/bin/qt53-env.sh || yes

# ffmpeg
if [ ! -e "libs" ]; then mkdir libs; fi
if [ ! -e "ffmpeg" ]; then mkdir ffmpeg; fi
#
cd libs/
export PREFIX_DIR="$PWD"
#
cd ../ffmpeg
wget http://ffmpeg.org/releases/ffmpeg-2.8.5.tar.bz2
tar xf ffmpeg*
cd ffmpeg*
# enabled:
# v4l2 -> webcam
# x11grab_xcb -> screen grabbing
# demuxers, decoders and parsers needed for webcams:
# mjpeg, h264

./configure --prefix="$PREFIX_DIR" \
    --disable-avfilter \
    --disable-avresample \
    --disable-bzlib \
    --disable-bsfs \
    --disable-dct \
    --disable-decoders \
    --disable-demuxers \
    --disable-doc \
    --disable-dwt \
    --disable-encoders \
    --disable-faan \
    --disable-fft \
    --disable-filters \
    --disable-iconv \
    --disable-indevs \
    --disable-lsp \
    --disable-lzma \
    --disable-lzo \
    --disable-mdct \
    --disable-muxers \
    --disable-network \
    --disable-outdevs \
    --disable-parsers \
    --disable-postproc \
    --disable-programs \
    --disable-protocols \
    --disable-rdft \
    --disable-sdl \
    --disable-static \
    --disable-swresample \
    --disable-swscale-alpha \
    --disable-vaapi \
    --disable-vdpau \
    --disable-xlib \
    --disable-yasm \
    --disable-zlib \
    --enable-shared \
    --enable-memalign-hack \
    --enable-indev=v4l2 \
    --enable-indev=x11grab_xcb \
    --enable-demuxer=h264 \
    --enable-demuxer=mjpeg \
    --enable-parser=h264 \
    --enable-parser=mjpeg \
    --enable-decoder=h264 \
    --enable-decoder=mjpeg

make -j$(nproc)
make install
cd ../../
# libsodium
git clone git://github.com/jedisct1/libsodium.git
cd libsodium
git checkout tags/1.0.8
./autogen.sh
./configure && make -j$(nproc)
sudo checkinstall --install --pkgname libsodium --pkgversion 1.0.8 --nodoc -y
sudo ldconfig
cd ..
# toxcore
git clone --branch v0.1.0 --depth=1 https://github.com/toktok/c-toxcore.git toxcore
cd toxcore
autoreconf -if
./configure
make -j$(nproc) > /dev/null
sudo make install
echo '/usr/local/lib/' | sudo tee -a /etc/ld.so.conf.d/locallib.conf
sudo ldconfig
cd ..

$CC --version
$CXX --version

# needed, otherwise ffmpeg doesn't get detected
export PKG_CONFIG_PATH="$PWD/libs/lib/pkgconfig"

# first build qTox without support for optional dependencies
echo '*** BUILDING "MINIMAL" VERSION ***'
qmake qtox.pro QMAKE_CC="$CC" QMAKE_CXX="$CXX" ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND=NO ENABLE_SYSTRAY_GTK_BACKEND=NO DISABLE_PLATFORM_EXT=YES SMILEYS=NO
# ↓ reduce if build fails with OOM
make -j$(nproc)
# clean it up, and build normal version
make clean
echo '*** BUILDING "FULL" VERSION ***'
qmake qtox.pro QMAKE_CC="$CC" QMAKE_CXX="$CXX"
# ↓ reduce if build fails with OOM
make -j$(nproc)
