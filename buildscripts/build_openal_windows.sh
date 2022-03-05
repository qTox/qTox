#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

usage()
{
    echo "Download and build openal for the windows cross compiling environment"
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

if [ "$ARCH" != "win32" ] && [ "$ARCH" != "win64" ]; then
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

"${SCRIPT_DIR}/download/download_openal.sh"

patch -p1 < "${SCRIPT_DIR}/patches/openal-cmake-3-11.patch"


if [ "${ARCH}" == "win64" ]; then
    MINGW_DIR="x86_64-w64-mingw32"
else
    MINGW_DIR="x86-w64-mingw32"
fi

export CFLAGS="-fPIC"
cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
    -DCMAKE_BUILD_TYPE=Release \
    -DALSOFT_UTILS=OFF \
    -DALSOFT_EXAMPLES=OFF \
    -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
    -DDSOUND_INCLUDE_DIR="/usr/${MINGW_DIR}/include" \
    -DDSOUND_LIBRARY="/usr/${MINGW_DIR}/lib/libdsound.a" \
    .

make -j $(nproc)
make install
