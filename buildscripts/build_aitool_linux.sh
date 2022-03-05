#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#    Copyright © 2019-2021 by The qTox Project Contributors

set -euo pipefail

"$(dirname "$(realpath "$0")")/download/download_aitool.sh"

cmake . -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON \
-DAPPIMAGEKIT_PACKAGE_DEBS=ON

make -j $(nproc)
make install
