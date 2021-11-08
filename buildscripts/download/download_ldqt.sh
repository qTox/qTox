#!/bin/bash

set -euo pipefail

LINUXDEPLOYQT_VERSION=570ca2dcb16f59ce0a2721f780093db8037cc9e0
LINUXDEPLOYQT_HASH=d53496c349f540ce17ea42c508d85802db1c7c3571e1bf962ec4e95482ea938e

source "$(dirname $0)"/common.sh

download_verify_extract_tarball \
    "https://github.com/probonopd/linuxdeployqt/archive/${LINUXDEPLOYQT_VERSION}.tar.gz" \
    "${LINUXDEPLOYQT_HASH}"
