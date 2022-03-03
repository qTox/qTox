#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/cross_compile_detection.sh"

usage()
{
    echo "Download and build sqlcipher for Windows or macOS"
    echo "Usage: $0 --arch {winx86_64|wini686|macos}"
}

parse_arch "$@"

"${SCRIPT_DIR}/download/download_sqlcipher.sh"

CFLAGS="-O2 -g0 -DSQLITE_HAS_CODEC -I$DEP_PREFIX/include/ $CROSS_CFLAG"
LDFLAGS="-lcrypto -L$DEP_PREFIX/lib/ $CROSS_LDFLAG"

if [ "$ARCH" == "macos" ]; then
    LIBS=''
else
    sed -i s/'if test "$TARGET_EXEEXT" = ".exe"'/'if test ".exe" = ".exe"'/g configure
    sed -i 's|exec $PWD/mksourceid manifest|exec $PWD/mksourceid.exe manifest|g' tool/mksqlite3h.tcl
    LIBS="-lgdi32 -lws2_32"
    LDFLAGS="$LDFLAGS -lgdi32"
fi

./configure "${HOST_OPTION}" \
            "--prefix=${DEP_PREFIX}" \
            --enable-shared \
            --disable-static \
            --enable-tempstore=yes \
            "CFLAGS=${CFLAGS}" \
            "LDFLAGS=${LDFLAGS}" \
            "LIBS=${LIBS}"

if [ "${ARCH}" != "macos" ]; then
    sed -i s/"TEXE = $"/"TEXE = .exe"/ Makefile
fi

make -j "${MAKE_JOBS}"
make install
