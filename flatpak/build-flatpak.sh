#!/usr/bin/env bash

# SPDX-License-Identifier: GPL-3.0+
#
# Copyright © 2018-2019 by The qTox Project Contributors
#
# This script should be run from the root of the repository

# usage: ./flatpak/build-flatpak.sh [Debug]
#
# If [Debug] is set to "Debug" the container will run in interactive mode and
# stay open to poke around in the filesystem.

readonly DEBUG="$1"

# Fail out on error
set -exo pipefail

if [ ! -f ./flatpak/build-flatpak.sh ]; then
    echo ""
    echo "You are attempting to run the build-flatpak.sh from a wrong directory."
    echo "If you wish to run this script, you'll have to have"
    echo "the repository root directory as the working directory."
    echo ""
    exit 1
fi

mkdir -p ./output

if [ "$DEBUG" == "Debug" ]
then
    echo "Execute: /qtox/appimage/build.sh to start the build script"
    echo "Execute: exit to leave the container"

    docker run --rm --privileged -it \
        -v $PWD:/qtox \
        -v $PWD/output:/output \
        debian:buster-slim \
        /bin/bash
else
    docker run --rm --privileged \
        -v $PWD:/qtox \
        -v $PWD/output:/output \
        debian:buster-slim \
        /bin/bash -c "/qtox/flatpak/build.sh"
fi

# use the version number in the name when building a tag on Travis CI
if [ -n "$TRAVIS_TAG" ]
then
    readonly OUTFILE=./output/qTox-"$TRAVIS_TAG".x86_64.flatpak
    mv ./output/*.flatpak "$OUTFILE"
    sha256sum "$OUTFILE" > "$OUTFILE".sha256
fi
