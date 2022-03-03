#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

source "${SCRIPT_DIR}/build_utils.sh"

parse_arch --dep "sodium" --supported "win32 win64 macos" "$@"

"${SCRIPT_DIR}/download/download_sodium.sh"

CFLAGS="${CROSS_CFLAG}" \
LDFLAGS="${CROSS_LDFLAG} -fstack-protector" \
  ./configure "${HOST_OPTION}" \
              "--prefix=${DEP_PREFIX}" \
              --enable-shared \
              --disable-static

make -j "${MAKE_JOBS}"
make install
