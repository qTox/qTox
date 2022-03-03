#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build sodium for Windows or macOS"
    echo "Usage: $0 --arch {winx86_64|wini686|macos}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_sodium.sh"

CFLAGS="${CROSS_CFLAG}" \
LDFLAGS="${CROSS_LDFLAG} -fstack-protector" \
  ./configure "${HOST_OPTION}" \
              "--prefix=${DEP_PREFIX}" \
              --enable-shared \
              --disable-static

make -j "${MAKE_JOBS}"
make install
