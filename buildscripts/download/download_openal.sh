#!/bin/bash

set -euo pipefail

OPENAL_VERSION=b80570bed017de60b67c6452264c634085c3b148
OPENAL_HASH=e9f6d37672e085d440ef8baeebb7d62fec1d152094c162e5edb33b191462bd78

source "$(dirname "$0")"/common.sh

## We can stop using the fork once OpenAL-Soft gets loopback capture implemented:
## https://github.com/kcat/openal-soft/pull/421
download_verify_extract_tarball \
    "https://github.com/irungentoo/openal-soft-tox/archive/${OPENAL_VERSION}.tar.gz" \
    "${OPENAL_HASH}"
