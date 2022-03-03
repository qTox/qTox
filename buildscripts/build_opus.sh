#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build opus for Windows or macOS"
    echo "Usage: $0 --arch {winx86_64|wini686|macos}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_opus.sh"

LDFLAGS="-fstack-protector ${CROSS_LDFLAG}" \
CFLAGS="-O2 -g0 ${CROSS_CFLAG}" \
CPPFLAGS="${CROSS_CPPFLAG}" \
    ./configure "${HOST_OPTION}" \
                             "--prefix=${DEP_PREFIX}" \
                             --enable-shared \
                             --disable-static \
                             --disable-extra-programs \
                             --disable-doc

make -j "${MAKE_JOBS}"
make install
