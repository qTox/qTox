#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build ffmpeg for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_ffmpeg.sh"

if [ "${ARCH}" == "x86_64" ]; then
    FFMPEG_ARCH="x86_64"
else
    FFMPEG_ARCH="x86"
fi

./configure --arch=${FFMPEG_ARCH} \
          --enable-gpl \
          --enable-shared \
          --disable-static \
          "--prefix=${DEP_PREFIX}" \
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

make -j "${MAKE_JOBS}"
make install
