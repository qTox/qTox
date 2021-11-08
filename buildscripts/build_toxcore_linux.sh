#!/bin/bash

set -euo pipefail

build_toxcore() {
    mkdir -p toxcore
    pushd toxcore >/dev/null || exit 1

    "$(dirname "$0")"/download/download_toxcore.sh

    cmake . \
        -DBOOTSTRAP_DAEMON=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        .

    make -j$(nproc)
    make -j9 install

    popd >/dev/null
}

build_toxext() {
    mkdir -p toxext
    pushd toxext >/dev/null || exit 1

    "$(dirname "$0")"/download/download_toxext.sh

    cmake . -DCMAKE_BUILD_TYPE=Release
    cmake --build . -- -j$(nproc)
    cmake --build . --target install

    popd >/dev/null
}

build_toxext_messages() {
    mkdir -p toxext_messages
    pushd toxext_messages > /dev/null || exit 1

    "$(dirname "$0")"/download/download_toxext_messages.sh

    cmake .  -DCMAKE_BUILD_TYPE=Release
    cmake --build . -- -j$(nproc)
    cmake --build . --target install

    popd >/dev/null
}

build_toxcore
build_toxext
build_toxext_messages
