#   Copyright © 2019 by The qTox Project Contributors
#
#   This file is part of qTox, a Qt-based graphical interface for Tox.
#   qTox is libre software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   qTox is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with qTox.  If not, see <http://www.gnu.org/licenses/>

#!/usr/bin/env bash

# SPDX-License-Identifier: GPL-3.0+
#
# Copyright © 2018 by The qTox Project Contributors
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
        debian:stretch-slim \
        /bin/bash
else
    docker run --rm --privileged \
        -v $PWD:/qtox \
        -v $PWD/output:/output \
        debian:stretch-slim \
        /bin/bash -c "/qtox/flatpak/build.sh"
fi

# use the version number in the name when building a tag on Travis CI
if [ -n "$TRAVIS_TAG" ]
then
    readonly OUTFILE=./output/qTox-"$TRAVIS_TAG".x86_64.flatpak
    mv ./output/*.flatpak "$OUTFILE"
    sha256sum "$OUTFILE" > "$OUTFILE".sha256
fi
