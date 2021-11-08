#!/bin/bash

set -euo pipefail

TOXCORE_VERSION=0.2.12
TOXCORE_HASH=30ae3263c9b68d3bef06f799ba9d7a67e3fad447030625f0ffa4bb22684228b0

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    https://github.com/TokTok/c-toxcore/archive/v$TOXCORE_VERSION.tar.gz \
    "$TOXCORE_HASH"
