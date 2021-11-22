#!/bin/bash

set -euo pipefail

"$(dirname "$0")"/download/download_aitool.sh

cmake . -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON \
-DAPPIMAGEKIT_PACKAGE_DEBS=ON

make -j $(nproc)
make install
