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
readonly FP_BUILD_DIR="$BUILD_DIR"/flatpak
readonly APT_FLAGS="-y --no-install-recommends"
# flatpak manifest file
readonly QTOX_MANIFEST="https://raw.githubusercontent.com/flathub/io.github.qtox.qTox/master/io.github.qtox.qTox.json"
# flatpak manifest download location
readonly MANIFEST_FILE="flatpak/io.github.qtox.qTox.json"
# directory containing necessary patches
readonly PATCH_DIR="flatpak/patches"
# use multiple cores when building
export MAKEFLAGS="-j$(nproc)"

# add backports repo, needed for a recent enough flatpak
echo "deb http://ftp.debian.org/debian stretch-backports main" > /etc/apt/sources.list.d/stretch-backports.list

# Get packages
apt-get update
apt-get install $APT_FLAGS ca-certificates git elfutils wget xz-utils patch

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

# build from the local build directory instead of the git repo
patch "$MANIFEST_FILE" < "$PATCH_DIR"/build_directory.patch

# this patch should contain all other patches needed
patch "$MANIFEST_FILE" < "$PATCH_DIR"/ci_fixes.patch

# create flatpak build directory
mkdir -p "$FP_BUILD_DIR"
cd "$FP_BUILD_DIR"

# Add 'https://flathub.org' remote:
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

# Build the qTox flatpak
flatpak-builder --disable-rofiles-fuse --install-deps-from=flathub --force-clean --repo=tox-repo qTox-flatpak "$QTOX_BUILD_DIR"/flatpak/io.github.qtox.qTox.json

# Create a bundle for distribution
flatpak build-bundle tox-repo "$OUTPUT_DIR"/qtox.flatpak io.github.qtox.qTox

# Chmod since everything is root:root
chmod 755 -R "$OUTPUT_DIR"
