#!/bin/bash

set -euo pipefail

OPUS_VERSION=1.3.1
OPUS_HASH=65b58e1e25b2a114157014736a3d9dfeaad8d41be1c8179866f144a2fb44ff9d

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    "https://archive.mozilla.org/pub/opus/opus-${OPUS_VERSION}.tar.gz" \
    "${OPUS_HASH}"
