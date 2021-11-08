#!/bin/bash

set -euo pipefail

"$(dirname "$0")"/download/download_snore.sh

cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_daemon=OFF \
      -DBUILD_settings=OFF \
      -DBUILD_snoresend=OFF \
      -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
      .

make -j $(nproc)
make install
