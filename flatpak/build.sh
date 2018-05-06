#!/usr/bin/env bash

# MIT License
#
# Copyright © 2018 by The qTox Project Contributors
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

# directory paths
readonly QTOX_SRC_DIR="/qtox"
readonly OUTPUT_DIR="/output"
readonly BUILD_DIR="/build"
readonly QTOX_BUILD_DIR="$BUILD_DIR"/qtox
readonly APT_FLAGS="-y --no-install-recommends"
# use multiple cores when building
export MAKEFLAGS="-j$(nproc)"

# add backports repo, needed for a recent enough flatpak
echo "deb http://ftp.debian.org/debian stretch-backports main" > /etc/apt/sources.list.d/stretch-backports.list

# Get packages
apt-get update
apt-get install $APT_FLAGS ca-certificates git elfutils

# install recent flatpak packages
apt-get install $APT_FLAGS -t stretch-backports flatpak flatpak-builder

# create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# copy qtox source
cp -r "$QTOX_SRC_DIR" "$QTOX_BUILD_DIR"
cd "$QTOX_BUILD_DIR"

# Add 'https://flathub.org' remote:
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

# Build the qTox flatpak
flatpak-builder --disable-rofiles-fuse --install-deps-from=flathub --force-clean --repo=tox-repo qTox-flatpak flatpak/chat.tox.qTox.json

# Create a bundle for distribution
flatpak build-bundle tox-repo "$OUTPUT_DIR"/qtox.flatpak chat.tox.qTox

# Chmod since everything is root:root
chmod 755 -R "$OUTPUT_DIR"
