#!/usr/bin/env bash

# MIT License
#
# Copyright © 2019 by The qTox Project Contributors
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

# usage: ./appimage/build-appimage.sh [Debug]
#
# If [Debug] is set to "Debug" the container will run in interactive mode and
# stay open to poke around in the filesystem.

readonly DEBUG="$1"

# Set this to True to upload the PR version of the 
# AppImage to transfer.sh for testing.
readonly UPLOAD_PR_APPIMAGE="False"

# Fail out on error
set -exo pipefail

# This script should be run from the root of the repository

if [ ! -f ./appimage/build-appimage.sh ]; then
    echo ""
    echo "You are attempting to run the build-appimage.sh from a wrong directory."
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

    docker run --rm -it \
        -v $PWD:/qtox \
        -v $PWD/output:/output \
        debian:stretch-slim \
        /bin/bash
else
    docker run --rm \
        -e CIRP_GITHUB_REPO_SLUG \
        -e TRAVIS_EVENT_TYPE \
        -e TRAVIS_COMMIT \
        -e TRAVIS_TAG \
        -v $PWD:/qtox \
        -v $PWD/output:/output \
        debian:stretch-slim \
        /bin/bash -c "/qtox/appimage/build.sh"
fi


# use the version number in the name when building a tag on Travis CI
if [ -n "$TRAVIS_TAG" ]
then
    # the aitool should have written the appimage in the same name
    # as below so no need to move things , it should have also written
    # the .zsync meta file as the given name below with .zsync
    # extension.
    readonly OUTFILE=./output/qTox-"$TRAVIS_TAG".x86_64.AppImage
 
    # just check if the files are in the right place
    eval "ls $OUTFILE"
    eval "ls $OUTFILE.zsync"

    sha256sum "$OUTFILE" > "$OUTFILE".sha256
else
    if [ "$UPLOAD_PR_APPIMAGE" == "True" ]
    then
        # upload PR builds to test them.
        echo "Uploading AppImage to transfer.sh"
        curl --upload-file "./output/qTox-$TRAVIS_COMMIT-x86_64.AppImage" \
            "https://transfer.sh/qTox-$TRAVIS_COMMIT-x86_64.AppImage" > ./upload
        echo "$(cat ./upload)"
        echo -n "$(cat ./upload)\\n" >> ./uploaded-to
        rm -rf ./upload ./uploaded-to
        sha256sum "./output/qTox-$TRAVIS_COMMIT-x86_64.AppImage"
    fi
fi
