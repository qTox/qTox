#!/usr/bin/env bash

set -euo pipefail

. .travis/cirp/check_precondition.sh
. .travis/cirp/install.sh

ci-release-publisher cleanup_publish
ci-release-publisher cleanup_store --scope current-build previous-finished-builds \
                                   --release complete incomplete
