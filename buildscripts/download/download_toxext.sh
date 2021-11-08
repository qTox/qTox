#!/bin/bash

set -euo pipefail

TOXEXT_VERSION=0.0.3
TOXEXT_HASH=99cf215d261a07bd83eafd1c69dcf78018db605898350b6137f1fd8e7c54734a

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    https://github.com/toxext/toxext/archive/v$TOXEXT_VERSION.tar.gz \
    "$TOXEXT_HASH"

