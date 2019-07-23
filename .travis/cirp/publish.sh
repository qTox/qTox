#!/usr/bin/env bash

set -euo pipefail

. .travis/cirp/check_precondition.sh

if [ ! -z "$TRAVIS_TEST_RESULT" ] && [ "$TRAVIS_TEST_RESULT" != "0" ]
then
  echo "Build has failed, skipping publishing"
  exit 0
fi

if [ "$#" != "1" ]
then
  echo "Error: No arguments provided. Please specify a directory containing artifacts as the first argument."
  exit 1
fi

ARTIFACTS_DIR="$1"

. .travis/cirp/check_cache.sh

. .travis/cirp/install.sh

ci-release-publisher publish --latest-release \
                             --latest-release-prerelease \
                             --latest-release-check-event-type cron \
                             --numbered-release \
                             --numbered-release-keep-count 3 \
                             --numbered-release-prerelease \
                             "$ARTIFACTS_DIR"

. .travis/cirp/update_cache.sh
