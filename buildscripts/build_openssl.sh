#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

source "${SCRIPT_DIR}/platform_detection.sh"

DEP_NAME="openssl"
parse_arch "$@"

if [[ "$SCRIPT_ARCH" == "win64" ]]; then
    OPENSSL_ARCH="mingw64"
else
    OPENSSL_ARCH="mingw"
fi

"${SCRIPT_DIR}/download/download_openssl.sh"

./Configure "--prefix=${DEP_PREFIX}" \
    "--openssldir=${DEP_PREFIX}/ssl" \
    shared \
    $OPENSSL_ARCH \
    --cross-compile-prefix=${CROSS_PREFIX}

make -j "${MAKE_JOBS}"
make install_sw
