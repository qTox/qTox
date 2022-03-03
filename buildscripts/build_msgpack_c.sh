#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2022 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

usage()
{
    echo "Download and build msgpack-c for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

"${SCRIPT_DIR}/download/download_msgpack_c.sh"

cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
    -DMSGPACK_BUILD_EXAMPLES=OFF \
    -DMSGPACK_BUILD_TESTS=OFF \
    .

make -j $(nproc)
make install
