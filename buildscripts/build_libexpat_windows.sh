#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

usage()
{
    echo "Download and build libexpat for windows"
    echo "Usage: $0 --arch {win64|win32}"
}

while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done

if [ "$ARCH" != "win32" ] && [ "$ARCH" != "win64" ]; then
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

"$(dirname $0)"/download/download_libexpat.sh

if [ "${ARCH}" == "win64" ]; then
    HOST="x86_64-w64-mingw32"
else
    HOST="i686-w64-mingw32"
fi

CFLAGS="-O2 -g0" ./configure --host="${HOST}" \
                                --prefix="/windows" \
                                --enable-static \
                                --disable-shared

make -j $(nproc)
make install
