#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build openssl for Windows or macOS"
    echo "Usage: $0 --arch {winx86_64|wini686|macos}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_openssl.sh"

if [[ "$ARCH" == "x86_64" ]]; then
    OPENSSL_ARCH="mingw64"
    CROSS_COMPILE_ARCH="--cross-compile-prefix=${ARCH}-w64-mingw32-"
elif [[ "$ARCH" == "i686" ]]; then
    OPENSSL_ARCH="mingw"
    CROSS_COMPILE_ARCH="--cross-compile-prefix=${ARCH}-w64-mingw32-"
elif [[ "$ARCH" == "macos" ]]; then
    OPENSSL_ARCH="darwin64-x86_64-cc"
    CROSS_COMPILE_ARCH=""
else
    echo "Invalid architecture"
    exit 1
fi

CFLAGS="${CROSS_CFLAG}" \
LDFLAGS="${CROSS_LDFLAG}" \
./Configure \
    "--prefix=${DEP_PREFIX}" \
    "--openssldir=${DEP_PREFIX}/ssl" \
    shared \
    ${CROSS_COMPILE_ARCH} \
    "${OPENSSL_ARCH}" \

make -j "${MAKE_JOBS}"
make install
