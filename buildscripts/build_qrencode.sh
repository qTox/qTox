#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build qrencode for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_qrencode.sh"

CFLAGS="-O2 -g0" ./configure --host="${ARCH}-w64-mingw32" \
                            --prefix=/windows \
                            --enable-shared \
                            --disable-static \
                            --disable-sdltest \
                            --without-tools \
                            --without-debug

make -j "${MAKE_JOBS}"
make install
