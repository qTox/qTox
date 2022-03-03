#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

source "${SCRIPT_DIR}/build_utils.sh"

parse_arch --dep "ffmpeg" --supported "win32 win64 macos" "$@"

if [ "${SCRIPT_ARCH}" == "win64" ]; then
    FFMPEG_ARCH="--arch=x86_64"
    TARGET_OS="--target-os=mingw32"
    CROSS_PREFIX="--cross-prefix=${MINGW_ARCH}-w64-mingw32-"
elif [ "${SCRIPT_ARCH}" == "win32" ]; then
    FFMPEG_ARCH="--arch=x86"
    TARGET_OS="--target-os=mingw32"
    CROSS_PREFIX="--cross-prefix=${MINGW_ARCH}-w64-mingw32-"
else
    FFMPEG_ARCH=""
    TARGET_OS=""
    CROSS_PREFIX=""
fi

"${SCRIPT_DIR}/download/download_ffmpeg.sh"

CFLAGS="${CROSS_CFLAG}" \
CPPFLAGS="${CROSS_CPPFLAG}" \
LDFLAGS="${CROSS_LDFLAG}" \
./configure ${FFMPEG_ARCH} \
          --enable-gpl \
          --enable-shared \
          --disable-static \
          "--prefix=${DEP_PREFIX}" \
          ${TARGET_OS} \
          ${CROSS_PREFIX} \
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
