#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build sqlcipher for Windows or macOS"
    echo "Usage: $0 --arch {winx86_64|wini686|macos}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_vpx.sh"

# There is a bug in gcc that breaks avx512 on 64-bit Windows https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
# VPX fails to build due to it.
# This is a workaround as suggested in https://stackoverflow.com/questions/43152633
if [ "${ARCH}" == "x86_64" ]; then
    ARCH_FLAGS="-fno-asynchronous-unwind-tables"
    TARGET="--target=x86_64-win64-gcc"
    CROSS_ARG="${ARCH}-w64-mingw32-"
elif [ "${ARCH}" == "i686" ]; then \
    ARCH_FLAGS=""
    TARGET="--target=x86-win32-gcc"
    CROSS_ARG="${ARCH}-w64-mingw32-"
elif [ "${ARCH}" == "macos" ]; then \
    ARCH_FLAGS=""
    TARGET=""
    CROSS_ARG=""
else
    exit 1
fi

if [ "${ARCH}" == "macos" ]; then
    patch -Np1 < "${SCRIPT_DIR}/patches/vpx-macos.patch"
else
    patch -Np1 < "${SCRIPT_DIR}/patches/vpx-windows.patch"
fi

CFLAGS="${ARCH_FLAGS} ${CROSS_CFLAG}" \
CPPFLAGS="${CROSS_CPPFLAG}" \
LDFLAGS="${CROSS_LDFLAG}" \
CROSS="${CROSS_ARG}" \
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

if [ "${ARCH}" == "macos" ]; then
    install_name_tool -id '@rpath/libvpx.dylib' ${DEP_PREFIX}/lib/libvpx.dylib
fi
