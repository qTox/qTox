#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "$SCRIPT_DIR"/cross_compile_detection.sh

usage()
{
    echo "Download and build libexif for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_libexif.sh"

CFLAGS="-O2 -g0 ${CROSS_CFLAG}" \
LDFLAGS="${CROSS_LDFLAG}" \
./configure "${HOST_OPTION}" \
                         "--prefix=${DEP_PREFIX}" \
                         --enable-shared \
                         --disable-static \
                         --disable-docs \
                         --disable-nls

make -j "${MAKE_JOBS}"
make install
