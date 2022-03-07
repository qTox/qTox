#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

source "${SCRIPT_DIR}/platform_detection.sh"

DEP_NAME="vpx"
parse_arch "$@"

if [[ "$SCRIPT_ARCH" == "win64" ]]; then
# There is a bug in gcc that breaks avx512 on 64-bit Windows https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
# VPX fails to build due to it.
# This is a workaround as suggested in https://stackoverflow.com/questions/43152633
    ARCH_FLAGS="-fno-asynchronous-unwind-tables"
    VPX_ARCH="x86_64-win64-gcc"
elif [[ "$SCRIPT_ARCH" == "win32" ]]; then
    ARCH_FLAGS=""
    VPX_ARCH="x86-win32-gcc"
else
    exit 1
fi

"${SCRIPT_DIR}/download/download_vpx.sh"

patch -Np1 < "${SCRIPT_DIR}/patches/vpx.patch"

CFLAGS=${ARCH_FLAGS} CROSS="${CROSS_PREFIX}" \
    ./configure --target="${VPX_ARCH}" \
        "--prefix=${DEP_PREFIX}" \
        --enable-shared \
        --disable-static \
        --enable-runtime-cpu-detect \
        --disable-examples \
        --disable-tools \
        --disable-docs \
        --disable-unit-tests

make -j "${MAKE_JOBS}"
make install
