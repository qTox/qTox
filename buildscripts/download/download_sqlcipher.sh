#!/bin/bash

set -euo pipefail

SQLCIPHER_VERSION=4.4.3
SQLCIPHER_HASH=b8df69b998c042ce7f8a99f07cf11f45dfebe51110ef92de95f1728358853133

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    "https://github.com/sqlcipher/sqlcipher/archive/v${SQLCIPHER_VERSION}.tar.gz" \
    "${SQLCIPHER_HASH}"
