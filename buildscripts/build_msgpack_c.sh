#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2022 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

source "${SCRIPT_DIR}/platform_detection.sh"

DEP_NAME="msgpack-c"
parse_arch "$@"

"${SCRIPT_DIR}/download/download_msgpack_c.sh"

cmake .\
    "-DCMAKE_INSTALL_PREFIX=${DEP_PREFIX}" \
    -DCMAKE_BUILD_TYPE=Release \
    "${CMAKE_TOOLCHAIN_FILE}" \
    -DMSGPACK_BUILD_EXAMPLES=OFF \
    -DMSGPACK_BUILD_TESTS=OFF \
    .

make -j "${MAKE_JOBS}"
make install
