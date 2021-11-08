#!/bin/bash

set -euo pipefail

"$(dirname "$0")"/download/download_ldqt.sh

qmake
make -j $(nproc)
make install
