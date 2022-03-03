#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

source "${SCRIPT_DIR}/build_utils.sh"

parse_arch --dep "openssl" --supported "win32 win64 macos" "$@"

if [[ "$SCRIPT_ARCH" == "win64" ]]; then
    OPENSSL_ARCH="mingw64"
    CROSS_COMPILE_ARCH="--cross-compile-prefix=${MINGW_ARCH}-w64-mingw32-"
elif [[ "$SCRIPT_ARCH" == "win32" ]]; then
    OPENSSL_ARCH="mingw"
    CROSS_COMPILE_ARCH="--cross-compile-prefix=${MINGW_ARCH}-w64-mingw32-"
elif [[ "$SCRIPT_ARCH" == "macos" ]]; then
    OPENSSL_ARCH="darwin64-x86_64-cc"
    CROSS_COMPILE_ARCH=""
fi

"${SCRIPT_DIR}/download/download_openssl.sh"

CFLAGS="${CROSS_CFLAG}" \
LDFLAGS="${CROSS_LDFLAG}" \
./Configure \
    "--prefix=${DEP_PREFIX}" \
    "--openssldir=${DEP_PREFIX}/ssl" \
    shared \
    ${CROSS_COMPILE_ARCH} \
    "${OPENSSL_ARCH}" \

make -j "${MAKE_JOBS}"
make install_sw
