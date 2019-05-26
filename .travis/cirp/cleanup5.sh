#!/usr/bin/env bash

set -euo pipefail

. .travis/cirp/check_precondition.sh

if [ -z "$TRAVIS_TEST_RESULT" ] && [ "$TRAVIS_TEST_RESULT" == "0" ]
then
  echo "Build has not failed, skipping cleanup"
  exit 0
fi

. .travis/cirp/install.sh

ci-release-publisher cleanup_publish
ci-release-publisher cleanup_store --scope current-build previous-finished-builds \
                                   --release complete incomplete
