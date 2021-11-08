#!/bin/bash

set -euo pipefail

TOXEXT_MESSAGES_VERSION=0.0.3
TOXEXT_MESSAGES_HASH=e7a9a199a3257382a85a8e555b6c8c540b652a11ca9a471b9da2a25a660dfdc3

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    https://github.com/toxext/tox_extension_messages/archive/v$TOXEXT_MESSAGES_VERSION.tar.gz \
    "$TOXEXT_MESSAGES_HASH"

