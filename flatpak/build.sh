#!/usr/bin/env bash

# SPDX-License-Identifier: GPL-3.0+
#
#   Copyright © 2018-2019 by The qTox Project Contributors

# Fail out on error
set -exuo pipefail

# directory paths
readonly QTOX_SRC_DIR="/qtox"
readonly OUTPUT_DIR="/output"
readonly BUILD_DIR="/build"
readonly QTOX_BUILD_DIR="$BUILD_DIR"/qtox
readonly FP_BUILD_DIR="$BUILD_DIR"/flatpak
readonly APT_FLAGS="-y --no-install-recommends"
# flatpak manifest download location
readonly MANIFEST_FILE="flatpak/io.github.qtox.qTox.json"
# directory containing necessary patches
readonly PATCH_DIR="flatpak/patches"
# use multiple cores when building
export MAKEFLAGS="-j$(nproc)"

# Get packages
apt-get update
apt-get install $APT_FLAGS ca-certificates git elfutils wget xz-utils patch bzip2 librsvg2-2 librsvg2-common flatpak flatpak-builder

# create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# copy qtox source
cp -r "$QTOX_SRC_DIR" "$QTOX_BUILD_DIR"
cd "$QTOX_BUILD_DIR"

# create flatpak build directory
mkdir -p "$FP_BUILD_DIR"
cd "$FP_BUILD_DIR"

# Add 'https://flathub.org' remote:
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

## Workaround for Flathub download issues: https://github.com/flathub/flathub/issues/845
# Pre download org.kde.Sdk because it fails often
for i in {1..5}
do
    echo "Download try $i"
    flatpak --system install flathub -y org.kde.Sdk/x86_64/5.14 || true
done
## Workaround end

# Build the qTox flatpak
flatpak-builder --disable-rofiles-fuse --install-deps-from=flathub --force-clean --repo=tox-repo qTox-flatpak "$QTOX_BUILD_DIR"/flatpak/io.github.qtox.qTox.json

# Create a bundle for distribution
flatpak build-bundle tox-repo "$OUTPUT_DIR"/qtox.flatpak io.github.qtox.qTox

# Chmod since everything is root:root
chmod 755 -R "$OUTPUT_DIR"
