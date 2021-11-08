#!/bin/bash

set -euo pipefail

SODIUM_VERSION=1.0.18
SODIUM_HASH=6f504490b342a4f8a4c4a02fc9b866cbef8622d5df4e5452b46be121e46636c1

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    "https://download.libsodium.org/libsodium/releases/libsodium-${SODIUM_VERSION}.tar.gz" \
    "${SODIUM_HASH}"
