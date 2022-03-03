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
    echo "Usage: $0 --arch {winx86_64|wini686}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_qrencode.sh"

cmake . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/windows \
    "${CMAKE_TOOLCHAIN_FILE}" \
    -DWITH_TOOLS=OFF \
    -DBUILD_SHARED_LIBS=ON

make -j "${MAKE_JOBS}"
make install
