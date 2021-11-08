#!/bin/bash

set -euo pipefail

SNORE_VERSION=0.7.0
SNORE_HASH=2e3f5fbb80ab993f6149136cd9a14c2de66f48cabce550dead167a9448f5bed9

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    "https://github.com/KDE/snorenotify/archive/v${SNORE_VERSION}.tar.gz" \
    "${SNORE_HASH}"
