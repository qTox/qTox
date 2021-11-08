#!/bin/bash

set -euo pipefail

OPENSSL_VERSION=1.1.1l
OPENSSL_HASH=0b7a3e5e59c34827fe0c3a74b7ec8baef302b98fa80088d7f9153aa16fa76bd1

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    "https://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz" \
    "$OPENSSL_HASH"
