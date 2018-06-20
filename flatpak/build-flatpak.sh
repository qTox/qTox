#!/usr/bin/env bash

# SPDX-License-Identifier: GPL-3.0+
#
# Copyright © 2018 by The qTox Project Contributors
#
# This script should be run from the root of the repository

if [ ! -f ./flatpak/build-flatpak.sh ]; then
    echo ""
    echo "You are attempting to run the build-flatpak.sh from a wrong directory."
    echo "If you wish to run this script, you'll have to have"
    echo "the repository root directory as the working directory."
    echo ""
    exit 1
fi

mkdir -p ./output

docker run --rm -it --privileged \
    -v $PWD:/qtox \
    -v $PWD/output:/output \
    debian:stretch-slim \
    /bin/bash #-c "/qtox/flatpak/build.sh"
