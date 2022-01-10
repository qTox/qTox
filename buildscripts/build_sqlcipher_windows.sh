#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

usage()
{
    echo "Download and build sqlcipher for the windows cross compiling environment"
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

"$(dirname "$0")"/download/download_sqlcipher.sh

sed -i s/'if test "$TARGET_EXEEXT" = ".exe"'/'if test ".exe" = ".exe"'/g configure
sed -i 's|exec $PWD/mksourceid manifest|exec $PWD/mksourceid.exe manifest|g' tool/mksqlite3h.tcl

./configure --host="${ARCH}-w64-mingw32" \
            --prefix=/windows/ \
            --enable-shared \
            --disable-static \
            --enable-tempstore=yes \
            CFLAGS="-O2 -g0 -DSQLITE_HAS_CODEC -I/windows/include/" \
            LDFLAGS="-lcrypto -lgdi32 -L/windows/lib/" \
            LIBS="-lgdi32 -lws2_32"

sed -i s/"TEXE = $"/"TEXE = .exe"/ Makefile

make -j $(nproc)
make install
