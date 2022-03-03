#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

source "${SCRIPT_DIR}/build_utils.sh"

parse_arch --dep "openal" --supported "win32 win64 macos" "$@"

"${SCRIPT_DIR}/download/download_openal.sh"

if [ "${SCRIPT_ARCH}" != "macos" ]; then
    patch -p1 < "${SCRIPT_DIR}/patches/openal-cmake-3-11.patch"
    DDSOUND="-DDSOUND_INCLUDE_DIR=/usr/${MINGW_ARCH}-w64-mingw32/include \
    -DDSOUND_LIBRARY=/usr/${MINGW_ARCH}-w64-mingw32/lib/libdsound.a"
    MACOSX_RPATH=""
else
    DDSOUND=""
    MACOSX_RPATH="-DCMAKE_MACOSX_RPATH=ON"
fi

export CFLAGS="-fPIC"
cmake "-DCMAKE_INSTALL_PREFIX=${DEP_PREFIX}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DALSOFT_UTILS=OFF \
    -DALSOFT_EXAMPLES=OFF \
    "${CMAKE_TOOLCHAIN_FILE}" \
    "${DDSOUND}" \
    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${MACOS_MINIMUM_SUPPORTED_VERSION}" \
    "${MACOSX_RPATH}" \
    .

make -j "${MAKE_JOBS}"
make install
