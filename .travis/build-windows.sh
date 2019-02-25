#!/usr/bin/env bash

# MIT License
#
# Copyright (c) 2017 Maxim Biro <nurupo.contributions@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# Fail out on error
set -exuo pipefail

readonly ARCH="$1"
readonly BUILD_TYPE="$2"
readonly CACHE_DIR="$3"
readonly STAGE="$4"

if [ -z "$ARCH" ]
then
  echo "Error: No architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the first argument to the script."
  exit 1
fi

if [[ "$ARCH" != "i686" ]] && [[ "$ARCH" != "x86_64" ]]
then
  echo "Error: Incorrect architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the first argument to the script."
  exit 1
fi

if [ -z "$BUILD_TYPE" ]
then
  echo "Error: No build type was specified. Please specify either 'release' or 'debug', case sensitive, as the second argument to the script."
  exit 1
fi

if [[ "$BUILD_TYPE" != "release" ]] && [[ "$BUILD_TYPE" != "debug" ]]
then
  echo "Error: Incorrect build type was specified. Please specify either 'release' or 'debug', case sensitive, as the second argument to the script."
  exit 1
fi

if [ -z "$CACHE_DIR" ]
then
  echo "Error: No cache directory path was specified. Please specify absolute path to the cache directory as the third argument to the script."
  exit 1
fi

if [ -z "$STAGE" ]
then
  echo "Error: No stage was specified. Please specify either 'stage1', 'stage2' or 'stage3' as the fourth argument to the script."
  exit 1
fi

if [[ "$STAGE" != "stage1" ]] && [[ "$STAGE" != "stage2" ]] && [[ "$STAGE" != "stage3" ]]
then
  echo "Error: Incorrect stage was specified. Please specify either 'stage1', 'stage2' or 'stage3', case sensitive, as the fourth argument to the script."
  exit 1
fi

# make the build stage visible to the deploy process
touch "$STAGE"

# make the build type visible to the deploy process
touch "$BUILD_TYPE"

# for debugging of the stage files
echo $PWD

# Just make sure those exist, makes logic easier
mkdir -p "$CACHE_DIR"
touch "$CACHE_DIR"/hash
mkdir -p workspace/"$ARCH"/dep-cache

# Purely for debugging
ls -lbh "$CACHE_DIR"


# If build.sh has changed, i.e. its hash doesn't match the previously stored one, and it's Stage 1
# Then we want to rebuild everything from scratch
if [ "`cat $CACHE_DIR/hash`" != "`sha256sum windows/cross-compile/build.sh`" ] && [ "$STAGE" == "stage1" ]
then
  # Clear the cache, removing all the pre-built dependencies
  rm -rf "$CACHE_DIR"/*
  touch "$CACHE_DIR"/hash
else
  # Copy over all pre-built dependencies
  cp -dR "$CACHE_DIR"/* workspace/"$ARCH"/dep-cache
fi

# Purely for debugging
ls -lbh "$CACHE_DIR"

# Purely for debugging
ls -lbh "$PWD"

# Build
sudo docker run --rm \
                -v "$PWD/workspace":/workspace \
                -v "$PWD":/qtox \
                -e TRAVIS_CI_STAGE="$STAGE" \
                debian:stretch-slim \
                /bin/bash /qtox/windows/cross-compile/build.sh "$ARCH" "$BUILD_TYPE"

# Purely for debugging
ls -lbh workspace/"$ARCH"/dep-cache/

# If we were building deps and it's any of the dependency building stages (Stage 1 or 2), copy over all the built dependencies to Travis cache
if [ "`cat $CACHE_DIR/hash`" != "`sha256sum windows/cross-compile/build.sh`" ] && ( [ "$STAGE" == "stage1" ] || [ "$STAGE" == "stage2" ] )
then
  # Clear out the cache
  rm -rf "$CACHE_DIR"/*
  touch "$CACHE_DIR"/hash
  cp -dR workspace/"$ARCH"/dep-cache/* "$CACHE_DIR"
fi

# Update the hash
if [ "`cat $CACHE_DIR/hash`" != "`sha256sum windows/cross-compile/build.sh`" ] && [ "$STAGE" == "stage2" ]
then
  sha256sum windows/cross-compile/build.sh > "$CACHE_DIR"/hash
fi

# Generate checksum files for releases
if [ "$STAGE" == "stage3" ] && [ "$BUILD_TYPE" == "release" ]
then
  readonly OUT_DIR=./workspace/"$ARCH"/qtox/"$BUILD_TYPE"/
  readonly NAME=setup-qtox-"$ARCH"-"$BUILD_TYPE".exe
  sha256sum "$OUT_DIR""$NAME" > "$OUT_DIR""$NAME".sha256
fi

# Purely for debugging
touch "$CACHE_DIR"/"$STAGE"
ls -lbh "$CACHE_DIR"
