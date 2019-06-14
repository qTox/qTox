#!/usr/bin/env bash

set -euo pipefail

. .travis/cirp/check_precondition.sh
. .travis/cirp/install.sh

ci-release-publisher cleanup_store --scope current-build \
                                   --release complete \
                                   --on-nonallowed-failure
