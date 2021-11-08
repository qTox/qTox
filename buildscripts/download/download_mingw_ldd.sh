#!/bin/bash

set -euo pipefail

MINGW_LDD_VERSION=0.2.1
MINGW_LDD_HASH=60d34506d2f345e011b88de172ef312f37ca3ba87f3764f511061b69878ab204

source "$(dirname $0)"/common.sh

download_verify_extract_tarball \
    "https://github.com/nurupo/mingw-ldd/archive/v${MINGW_LDD_VERSION}.tar.gz" \
    "${MINGW_LDD_HASH}"
