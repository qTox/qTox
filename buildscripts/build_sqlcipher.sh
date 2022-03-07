#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

source "${SCRIPT_DIR}/build_utils.sh"

parse_arch --dep "sqlcipher" --supported "win32 win64" "$@"

"${SCRIPT_DIR}/download/download_sqlcipher.sh"

sed -i s/'if test "$TARGET_EXEEXT" = ".exe"'/'if test ".exe" = ".exe"'/g configure
sed -i 's|exec $PWD/mksourceid manifest|exec $PWD/mksourceid.exe manifest|g' tool/mksqlite3h.tcl

./configure "${HOST_OPTION}" \
            "--prefix=${DEP_PREFIX}" \
            --enable-shared \
            --disable-static \
            --enable-tempstore=yes \
            CFLAGS="-O2 -g0 -DSQLITE_HAS_CODEC -I${DEP_PREFIX}/include/" \
            LDFLAGS="-lcrypto -lgdi32 -L${DEP_PREFIX}/lib/" \
            LIBS="-lgdi32 -lws2_32"

sed -i s/"TEXE = $"/"TEXE = .exe"/ Makefile

make -j "${MAKE_JOBS}"
make install
