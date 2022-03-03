#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build vpx for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_vpx.sh"

# There is a bug in gcc that breaks avx512 on 64-bit Windows https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
# VPX fails to build due to it.
# This is a workaround as suggested in https://stackoverflow.com/questions/43152633
if [ "${ARCH}" == "x86_64" ]; then
    ARCH_FLAGS="-fno-asynchronous-unwind-tables"
    VPX_ARCH="x86_64-win64-gcc"
elif [ "${ARCH}" == "i686" ]; then \
    ARCH_FLAGS=""
    VPX_ARCH="x86-win32-gcc"
else
    exit 1
fi

patch -Np1 < "$(dirname "$0")"/patches/vpx-windows.patch

CFLAGS=${ARCH_FLAGS} CROSS="${CROSS_ARG}" \
    ./configure \
        ${TARGET} \
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
