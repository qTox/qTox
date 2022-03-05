#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

"$(dirname "$(realpath "$0")")/download/download_snore.sh"

cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_daemon=OFF \
      -DBUILD_settings=OFF \
      -DBUILD_snoresend=OFF \
      -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
      .

make -j $(nproc)
make install
