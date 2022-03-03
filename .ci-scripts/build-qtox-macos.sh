#!/bin/bash

#    Copyright © 2016-2021 by The qTox Project Contributors
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

# Fail out on error
set -eu -o pipefail

readonly BIN_NAME="qTox.dmg"

SCRIPT_DIR=$(dirname $(realpath "$0"))

source "${SCRIPT_DIR}/../buildscripts/build_utils.sh"

# kind of a hack.. but we want to extract DEP_PREFIX from it
parse_arch --arch macos --supported macos --dep macos

if [ "$1" == "user" ]; then
    DEPLOYMENT_TARGET=""
    PREFIX_PATH="$(brew --prefix qt@5)"
elif [ "$1" == "dist" ]; then
    DEPLOYMENT_TARGET="-DCMAKE_OSX_DEPLOYMENT_TARGET=$MACOS_MINIMUM_SUPPORTED_VERSION"
    PREFIX_PATH="$DEP_PREFIX;$(brew --prefix qt@5)"
else
    echo "Unknown arg $1"
    exit 1
fi

build_qtox() {
    cmake -DUPDATE_CHECK=ON \
        -DSPELL_CHECK=OFF \
        -DSTRICT_OPTIONS=ON \
        -DCMAKE_BUILD_TYPE=Release \
        "${DEPLOYMENT_TARGET}" \
        "-DCMAKE_PREFIX_PATH=${PREFIX_PATH}" .
    make -j$(sysctl -n hw.ncpu)
    export CTEST_OUTPUT_ON_FAILURE=1
    ctest -j$(sysctl -n hw.ncpu)
    make install
}

check() {
    if [[ ! -s "$BIN_NAME" ]]
    then
        echo "There's no $BIN_NAME!"
        exit 1
    fi
}

make_hash() {
    shasum -a 256 "$BIN_NAME" > "$BIN_NAME".sha256
}

main() {
    build_qtox
    check
    make_hash
}
main
