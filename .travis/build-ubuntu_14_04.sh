#!/bin/bash
#
#    Copyright © 2015-2016 by The qTox Project
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


# Qt 5.3, since that's the lowest supported version
sudo add-apt-repository -y ppa:beineri/opt-qt532-trusty
sudo apt-get update -qq

# install needed Qt, OpenAL, opus, qrencode, GTK tray deps, sqlcipher
sudo apt-get install -y build-essential \
    qt53base \
    qt53script \
    qt53svg \
    qt53tools \
    qt53xmlpatterns \
    libopenal-dev \
    libxss-dev qrencode \
    libqrencode-dev \
    libglib2.0-dev \
    libgdk-pixbuf2.0-dev \
    libgtk2.0-dev \
    libsqlcipher-dev \
    libtool \
    autotools-dev \
    automake \
    checkinstall \
    check \
    libopus-dev \
    libvpx-dev

# Qt
source /opt/qt53/bin/qt53-env.sh

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
./configure --prefix="$PREFIX_DIR" --enable-shared --disable-static --disable-programs --disable-protocols --disable-doc --disable-sdl --disable-avfilter --disable-avresample --disable-filters --disable-iconv --disable-network --disable-muxers --disable-postproc --disable-swresample --disable-swscale-alpha --disable-dct --disable-dwt --disable-lsp --disable-lzo --disable-mdct --disable-rdft --disable-fft --disable-faan --disable-vaapi --disable-vdpau --disable-zlib --disable-xlib --disable-bzlib --disable-lzma --disable-encoders --disable-yasm --enable-memalign-hack
make -j$(nproc)
make install
cd ../../
# filter_audio
git clone https://github.com/irungentoo/filter_audio
cd filter_audio
make -j$(nproc)
sudo make install
cd ..
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
git clone https://github.com/irungentoo/toxcore.git
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
# first build qTox without support for optional dependencies
echo '*** BUILDING "MINIMAL" VERSION ***'
qmake qtox.pro QMAKE_CC="$CC" QMAKE_CXX="$CXX" DISABLE_FILTER_AUDIO=YES ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND=NO ENABLE_SYSTRAY_GTK_BACKEND=NO DISABLE_PLATFORM_EXT=YES
# ↓ with $(nproc) fails, since travis gives 32 threads, and it leads to OOM
make -j10
# clean it up, and build normal version
make clean
echo '*** BUILDING "FULL" VERSION ***'
qmake qtox.pro QMAKE_CC="$CC" QMAKE_CXX="$CXX" 
# ↓ with $(nproc) fails, since travis gives 32 threads, and it leads to OOM
make -j10
