#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

usage()
{
    echo "Download and build ffmpeg for the windows cross compiling environment"
    echo "Usage: $0 --arch {win64|win32}"
}

ARCH=""

while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done

if [ "$ARCH" != "win32" ] && [ "$ARCH" != "win64" ]; then
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

"$(dirname "$0")"/download/download_ffmpeg.sh

if [ "${ARCH}" == "win64" ]; then
    FFMPEG_ARCH="x86_64"
    CROSS_PREFIX="x86_64-w64-mingw32-"
else
    FFMPEG_ARCH="x86"
    CROSS_PREFIX="i686-w64-mingw32-"
fi

./configure --arch=${FFMPEG_ARCH} \
          --enable-gpl \
          --enable-shared \
          --disable-static \
          --prefix=/windows/ \
          --target-os="mingw32" \
          --cross-prefix="${CROSS_PREFIX}" \
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
