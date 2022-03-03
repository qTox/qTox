#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

usage()
{
    echo "Download and build vpx for the windows cross compiling environment"
    echo "Usage: $0 --arch {win64|win32}"
}

ARCH=""

while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done


if [[ "$ARCH" == "win64" ]]; then
# There is a bug in gcc that breaks avx512 on 64-bit Windows https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412
# VPX fails to build due to it.
# This is a workaround as suggested in https://stackoverflow.com/questions/43152633
    ARCH_FLAGS="-fno-asynchronous-unwind-tables"
    VPX_ARCH="x86_64-win64-gcc"
    CROSS="x86_64-w64-mingw32-"
elif [[ "$ARCH" == "win32" ]]; then
    ARCH_FLAGS=""
    VPX_ARCH="x86-win32-gcc"
    CROSS="i686-w64-mingw32-"
else
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

"$(dirname "$0")"/download/download_vpx.sh

patch -Np1 < "$(dirname "$0")"/patches/vpx.patch

CFLAGS=${ARCH_FLAGS} CROSS="${CROSS}" \
    ./configure --target="${VPX_ARCH}" \
        --prefix=/windows/ \
        --enable-shared \
        --disable-static \
        --enable-runtime-cpu-detect \
        --disable-examples \
        --disable-tools \
        --disable-docs \
        --disable-unit-tests

make -j $(nproc)
make install
