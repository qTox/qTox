#!/usr/bin/env bash

if [ ! -z "$TRAVIS_TEST_RESULT" ] && [ "$TRAVIS_TEST_RESULT" != "0" ]
then
  echo "Build has failed, skipping updating the cache"
  exit 0
fi

mkdir -p /opt/cirp
git rev-parse HEAD > /opt/cirp/previous_runs_commit
