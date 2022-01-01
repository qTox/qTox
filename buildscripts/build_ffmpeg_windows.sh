#!/bin/bash

#    Copyright © 2021 by The qTox Project Contributors
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

usage()
{
    echo "Download and build ffmpeg for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

ARCH=""

while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done

if [ "$ARCH" != "i686" ] && [ "$ARCH" != "x86_64" ]; then
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

"$(dirname "$0")"/download/download_ffmpeg.sh

if [ "${ARCH}" == "x86_64" ]; then
    FFMPEG_ARCH="x86_64"
else
    FFMPEG_ARCH="x86"
fi

./configure --arch=${FFMPEG_ARCH} \
          --enable-gpl \
          --enable-shared \
          --disable-static \
          --prefix=/windows/ \
          --target-os="mingw32" \
          --cross-prefix="${ARCH}-w64-mingw32-" \
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

make -j $(nproc)
make install
