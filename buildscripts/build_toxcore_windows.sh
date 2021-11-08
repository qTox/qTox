#!/bin/bash

#    Copyright © 2021 by The qTox Project Contributors
#
#    This program is libre software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

    cmake --build . -- -j$(nproc)
    cmake --build . --target install

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
