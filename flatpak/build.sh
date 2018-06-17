#!/usr/bin/env bash

# SPDX-License-Identifier: GPL-3.0+
#
# Copyright © 2018 by The qTox Project Contributors

# Fail out on error
set -exuo pipefail

# directory paths
readonly QTOX_SRC_DIR="/qtox"
readonly OUTPUT_DIR="/output"
readonly BUILD_DIR="/build"
readonly QTOX_BUILD_DIR="$BUILD_DIR"/qtox
readonly APT_FLAGS="-y --no-install-recommends"
# flatpak manifest file
readonly QTOX_MANIFEST="https://raw.githubusercontent.com/flathub/io.github.qtox.qTox/master/io.github.qtox.qTox.json"
# flatpak manifest download location
readonly MANIFEST_FILE="flatpak/io.github.qtox.qTox.json"
# use multiple cores when building
export MAKEFLAGS="-j$(nproc)"

# add backports repo, needed for a recent enough flatpak
echo "deb http://ftp.debian.org/debian stretch-backports main" > /etc/apt/sources.list.d/stretch-backports.list

# Get packages
apt-get update
apt-get install $APT_FLAGS ca-certificates git elfutils wget xz-utils

# install recent flatpak packages
apt-get install $APT_FLAGS -t stretch-backports flatpak flatpak-builder

# create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# copy qtox source
cp -r "$QTOX_SRC_DIR" "$QTOX_BUILD_DIR"
cd "$QTOX_BUILD_DIR"

# download manifest file if not in repo, this allows an easy local override
if [ ! -f "$MANIFEST_FILE" ];
then
  wget -O "$MANIFEST_FILE" "$QTOX_MANIFEST"
fi

# Add 'https://flathub.org' remote:
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

# Build the qTox flatpak
flatpak-builder --disable-rofiles-fuse --install-deps-from=flathub --force-clean --repo=tox-repo qTox-flatpak flatpak/io.github.qtox.qTox.json type=dir path="$QTOX_SRC_DIR"

# Create a bundle for distribution
flatpak build-bundle tox-repo "$OUTPUT_DIR"/qtox.flatpak io.github.qtox.qTox

# Chmod since everything is root:root
chmod 755 -R "$OUTPUT_DIR"
