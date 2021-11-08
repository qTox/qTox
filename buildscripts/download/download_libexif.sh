#!/bin/bash

set -euo pipefail

LIBEXIF_VERSION=0.6.23
LIBEXIF_HASH=a740a99920eb81ae0aa802bb46e683ce6e0cde061c210f5d5bde5b8572380431

source "$(dirname $0)"/common.sh

download_verify_extract_tarball \
    "https://github.com/libexif/libexif/releases/download/v${LIBEXIF_VERSION}/libexif-${LIBEXIF_VERSION}.tar.xz" \
    "${LIBEXIF_HASH}"
