#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

source "${SCRIPT_DIR}/platform_detection.sh"

DEP_NAME="gdb"
parse_arch "$@"

"${SCRIPT_DIR}/download/download_gdb.sh"

CFLAGS="-O2 -g0" ./configure "${HOST_OPTION}" \
                                --prefix="${DEP_PREFIX}" \
                                --enable-static \
                                --disable-shared

make -j "${MAKE_JOBS}"
make install
