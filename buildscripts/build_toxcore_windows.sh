#!/bin/bash

set -euo pipefail

build_toxcore() {
    TOXCORE_SRC="$(realpath toxcore)"

    mkdir -p "$TOXCORE_SRC"
    pushd $TOXCORE_SRC >/dev/null || exit 1

    "$(dirname "$0")"/download/download_toxcore.sh

    cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
            -DBOOTSTRAP_DAEMON=OFF \
            -DCMAKE_BUILD_TYPE=Release \
            -DENABLE_STATIC=OFF \
            -DENABLE_SHARED=ON \
            -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
            .

    make -j$(nproc)
    make -j9 install

    popd >/dev/null
}

build_toxext() {
    TOXEXT_SRC="$(realpath toxext)"

    mkdir -p "$TOXEXT_SRC"
    pushd $TOXEXT_SRC >/dev/null || exit 1

    "$(dirname "$0")"/download/download_toxext.sh

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

    "$(dirname "$0")"/download/download_toxext_messages.sh

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
