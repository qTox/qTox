#!/usr/bin/env bash

# SPDX-License-Identifier: GPL-3.0+
#
#   Copyright © 2018-2019 by The qTox Project Contributors

# Fail out on error
set -exuo pipefail

# use multiple cores when building
export MAKEFLAGS="-j$(nproc)"
FLATPAK_DESCRIPTOR=$(dirname $(realpath $0))/io.github.qtox.qTox.json

mkdir -p /flatpak-build
cd /flatpak-build

# Build the qTox flatpak
flatpak-builder --disable-rofiles-fuse --install-deps-from=flathub --force-clean --repo=tox-repo build "$FLATPAK_DESCRIPTOR"

# Create a bundle for distribution
flatpak build-bundle tox-repo qtox.flatpak io.github.qtox.qTox

cp qtox.flatpak /qtox

