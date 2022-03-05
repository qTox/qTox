#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath "$0"))

build_toxcore() {
    TOXCORE_SRC="$(realpath toxcore)"

    mkdir -p "$TOXCORE_SRC"
    pushd $TOXCORE_SRC >/dev/null || exit 1

    "${SCRIPT_DIR}/download/download_toxcore.sh"

    cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
            -DBOOTSTRAP_DAEMON=OFF \
            -DCMAKE_BUILD_TYPE=Release \
            -DENABLE_STATIC=OFF \
            -DENABLE_SHARED=ON \
            -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
            .

    cmake --build . -- -j$(nproc)
    cmake --build . --target install

    popd >/dev/null
}

build_toxext() {
    TOXEXT_SRC="$(realpath toxext)"

    mkdir -p "$TOXEXT_SRC"
    pushd $TOXEXT_SRC >/dev/null || exit 1

    "${SCRIPT_DIR}/download/download_toxext.sh"

    cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
            .

    cmake --build . -- -j$(nproc)
    cmake --build . --target install

    popd >/dev/null
}

build_toxext_messages() {
    TOXEXT_MESSAGES_SRC="$(realpath toxext_messages)"

    mkdir -p "$TOXEXT_MESSAGES_SRC"
    pushd $TOXEXT_MESSAGES_SRC > /dev/null || exit 1

    "${SCRIPT_DIR}/download/download_toxext_messages.sh"

    cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
            .
    cmake --build . --target install

    popd >/dev/null
}

build_toxcore
build_toxext
build_toxext_messages
