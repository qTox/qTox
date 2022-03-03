#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build openal for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_openal.sh"

if [ "${ARCH}" != "macos" ]; then
    patch -p1 < "${SCRIPT_DIR}/patches/openal-cmake-3-11.patch"
    DDSOUND="-DDSOUND_INCLUDE_DIR=/usr/${ARCH}-w64-mingw32/include \
    -DDSOUND_LIBRARY=/usr/${ARCH}-w64-mingw32/lib/libdsound.a \
    "
else
    DDSOUND=""
fi

export CFLAGS="-fPIC"
cmake "-DCMAKE_INSTALL_PREFIX=${DEP_PREFIX}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DALSOFT_UTILS=OFF \
    -DALSOFT_EXAMPLES=OFF \
    "${CMAKE_TOOLCHAIN_FILE}" \
    "${DDSOUND}" \
    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${MACOS_MINIMUM_SUPPORTED_VERSION}" \
    .

make -j "${MAKE_JOBS}"
make install
