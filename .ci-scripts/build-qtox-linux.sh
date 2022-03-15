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

usage() {
    echo "$0 [--minimal|--full] --build-type [Debug|Release]"
    echo "Build script to build/test qtox from a CI environment."
    echo "--minimal or --full are requied, --build-type is required."
}

while (( $# > 0 )); do
    case $1 in
    --minimal) MINIMAL=1 ; shift ;;
    --full) MINIMAL=0; shift ;;
    --sanitize) SANITIZE=1; shift ;;
    --tidy) TIDY=1; shift;;
    --build-type) BUILD_TYPE=$2; shift 2 ;;
    --help|-h) usage; exit 1 ;;
    *) echo "Unexpected argument $1"; usage; exit 1 ;;
    esac
done

if [ -z "${MINIMAL+x}" ]; then
    echo "Please build either minimal or full version of qtox"
    usage
    exit 1
fi

if [ -z "${BUILD_TYPE+x}" ]; then
    echo "Please spedify build type"
    usage
    exit 1
fi

SANITIZE_ARGS=""
if [ ! -z ${SANITIZE+x} ]; then
    SANITIZE_ARGS="-DASAN=ON"
fi

if [ ! -z ${TIDY+x} ]; then
    export CXX=clang++
    export CC=clang
    COMPILE_COMMANDS="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
else
    COMPILE_COMMANDS=""
fi


SRCDIR=/qtox
export CTEST_OUTPUT_ON_FAILURE=1

if [ $MINIMAL -eq 1 ]; then
    cmake "$SRCDIR" \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DSMILEYS=DISABLED \
        -DSTRICT_OPTIONS=ON \
        -DSPELL_CHECK=OFF \
        ${COMPILE_COMMANDS} \
        $SANITIZE_ARGS
else
    cmake "$SRCDIR" \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DUPDATE_CHECK=ON \
        -DSTRICT_OPTIONS=ON \
        -DCODE_COVERAGE=ON \
        -DDESKTOP_NOTIFICATIONS=ON \
        ${COMPILE_COMMANDS} \
        $SANITIZE_ARGS
fi

cmake --build . -- -j $(nproc)

if [ ! -z ${TIDY+x} ]; then
    run-clang-tidy -header-filter=.* src/ audio/src/ audio/include test/src/ \
        test/include util/src/ util/include/
else
    ctest -j$(nproc)
fi
