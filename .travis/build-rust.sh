#!/bin/bash
#
#    Copyright © 2016 by The qTox Project Contributors
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
#

# used in travis to:
#  - pull in dependencies for building libsodium
#  - build required libsodium

# Fail out on error
set -eu -o pipefail

build_libsodium() {
    sudo apt-get update -qq
    sudo apt-get install -y \
        build-essential \
        libtool \
        autotools-dev \
        automake \
        checkinstall \
        check \
        git \
        yasm \
        pkg-config || yes

    git clone https://github.com/jedisct1/libsodium.git libsodium
        --branch 1.0.11 \
        --depth 1

    cd libsodium
    ./autogen.sh
    ./configure && make -j$(nproc)
    sudo checkinstall --install --pkgname libsodium --pkgversion 1.0.11 --nodoc -y
    sudo ldconfig
    cd ..
}

build_rust_bits() {
    # TODO: make it a loop over paths once there are more rust bits
    cd tools/update-server/qtox-updater-sign
    cargo build --verbose
    cargo test --verbose
    # add `cargo doc` once it's needed?
}

main() {
    build_libsodium
    build_rust_bits
}
main
