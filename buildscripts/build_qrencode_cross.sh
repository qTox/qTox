#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

usage()
{
    echo "Download and build qrencode for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

ARCH=""

while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done

if [ "$ARCH" != "i686" ] && [ "$ARCH" != "x86_64" ]; then
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

"$(dirname "$0")"/download/download_qrencode.sh

cmake . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/windows \
    "${CMAKE_TOOLCHAIN_FILE}" \
    -DWITH_TOOLS=OFF \
    -DBUILD_SHARED_LIBS=ON

make -j $(nproc)
make install
