#!/bin/bash
#
#    Copyright © 2016 by The qTox Project Contributors
#
#    This program is libre software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Fail out on error
set -euo pipefail

# Just make sure those exists, makes logic easier
mkdir -p $DEP_CACHE
touch $DEP_CACHE/hash
mkdir -p workspace/$ARCH/dep-cache

# If build.sh has changed, i.e. its hash doesn't match the previously stored one, and it's Stage 1
# Then we want to rebuild everything from scratch
if [ "`cat $DEP_CACHE/hash`" != "`sha256sum windows/cross-compile/build.sh`" ] && [ "$TRAVIS_CI_STAGE_ONE" == "true" ]
then
  # Clear the cache, removing all the pre-built dependencies
  rm -rf $DEP_CACHE/*
  touch $DEP_CACHE/hash
else
  # Copy over all pre-built dependencies
  cp -a $DEP_CACHE/* workspace/$ARCH/dep-cache
fi

# Build
sudo docker run --rm \
                -v "$PWD/workspace":/workspace \
                -v "$PWD/windows/cross-compile":/script \
                -v "$PWD":/qtox \
                -e TRAVIS_CI_STAGE_ONE=$TRAVIS_CI_STAGE_ONE \
                -e TRAVIS_CI_STAGE_TWO=$TRAVIS_CI_STAGE_TWO \
                ubuntu:16.04 \
                /bin/bash /script/build.sh $ARCH $BUILD_TYPE

# If it's any of the dependency building stages (Stage 1 or 2), copy over all the built dependencies to Travis cache
if [ "$TRAVIS_CI_STAGE_ONE" == "true" ] || [ "$TRAVIS_CI_STAGE_TWO" == "true" ]
then
  cp -a workspace/$ARCH/dep-cache/* $DEP_CACHE
fi

# We update the hash only at the end of the Stage 2
if [ "`cat $DEP_CACHE/hash`" != "`sha256sum windows/cross-compile/build.sh`" ] && [ "$TRAVIS_CI_STAGE_TWO" == "true" ]
then
  sha256sum windows/cross-compile/build.sh > $DEP_CACHE/hash
fi
