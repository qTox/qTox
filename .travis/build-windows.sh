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
set -exuo pipefail

readonly ARCH=$1
readonly BUILD_TYPE=$2
readonly CACHE_DIR=$3
readonly STAGE=$4

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


# Just make sure those exists, makes logic easier
mkdir -p $CACHE_DIR
touch $CACHE_DIR/hash
mkdir -p workspace/$ARCH/dep-cache
sudo chown `id -u -n`:`id -g -n` -R $CACHE_DIR

rm -rf $CACHE_DIR/*
exit 0

# If build.sh has changed, i.e. its hash doesn't match the previously stored one, and it's Stage 1
# Then we want to rebuild everything from scratch
if [ "`cat $CACHE_DIR/hash`" != "`sha256sum windows/cross-compile/build.sh`" ] && [ "$STAGE" == "stage1" ]
then
  # Clear the cache, removing all the pre-built dependencies
  rm -rf $CACHE_DIR/*
  touch $CACHE_DIR/hash
else
  # Copy over all pre-built dependencies
  cp -a $CACHE_DIR/* workspace/$ARCH/dep-cache
fi

ls -lbh $CACHE_DIR

# Build
sudo docker run --rm \
                -v "$PWD/workspace":/workspace \
                -v "$PWD/windows/cross-compile":/script \
                -v "$PWD":/qtox \
                -e TRAVIS_CI_STAGE=$STAGE \
                ubuntu:16.04 \
                /bin/bash /script/build.sh $ARCH $BUILD_TYPE

# If it's any of the dependency building stages (Stage 1 or 2), copy over all the built dependencies to Travis cache
if [ "$STAGE" == "stage1" ] || [ "$STAGE" == "stage2" ]
then
  rm -rf $CACHE_DIR/*
  touch $CACHE_DIR/hash
  # Docker runs as root, so we chown back to out user to be able to cp files
  sudo chown `id -u -n`:`id -g -n` -R workspace
  cp -a workspace/$ARCH/dep-cache/* $CACHE_DIR
fi

# We update the hash only at the end of the Stage 2
if [ "`cat $CACHE_DIR/hash`" != "`sha256sum windows/cross-compile/build.sh`" ] && [ "$STAGE" == "stage2" ]
then
  sha256sum windows/cross-compile/build.sh > $CACHE_DIR/hash
fi

touch $CACHE_DIR/$STAGE
ls -lbh $CACHE_DIR

