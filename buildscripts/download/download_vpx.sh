#!/bin/bash

set -euo pipefail

VPX_VERSION=1.10.0
VPX_HASH=85803ccbdbdd7a3b03d930187cb055f1353596969c1f92ebec2db839fa4f834a

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    "https://github.com/webmproject/libvpx/archive/v$VPX_VERSION.tar.gz" \
    "${VPX_HASH}"
