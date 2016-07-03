#!/bin/bash
#
# Script that runs all the necessary tests on travis.
#
# Exits as soon as there's a failure in test.

set -e

if [[ "$TRAVIS_OS_NAME" == "linux" ]]
then
    # osx cannot into extended regexp for grep, thus verify only on Linux
    bash ./verify-commit-messages.sh "$TRAVIS_COMMIT_RANGE"
    bash ./.travis/build-ubuntu_14_04.sh
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]
then
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -i
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -b
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -d
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -dmg
fi
