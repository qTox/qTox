#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build openssl for the windows cross compiling environment"
    echo "Usage: $0 --arch {winx86_64|wini686}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_openssl.sh"

if [[ "$ARCH" == "x86_64" ]]; then
    OPENSSL_ARCH="mingw64"
elif [[ "$ARCH" == "i686" ]]; then
    OPENSSL_ARCH="mingw"
else
    echo "Invalid architecture"
    exit 1
fi

./Configure "--prefix=${DEP_PREFIX}" \
    "--openssldir=${DEP_PREFIX}/ssl" \
    shared \
    $OPENSSL_ARCH \
    "${CROSS_COMPILE_ARCH}"

make -j "${MAKE_JOBS}"
make install
