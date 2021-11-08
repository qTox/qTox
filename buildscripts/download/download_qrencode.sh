#!/bin/bash

set -euo pipefail

QRENCODE_VERSION=4.1.1
QRENCODE_HASH=e455d9732f8041cf5b9c388e345a641fd15707860f928e94507b1961256a6923

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    "https://fukuchi.org/works/qrencode/qrencode-${QRENCODE_VERSION}.tar.bz2" \
    "${QRENCODE_HASH}"
