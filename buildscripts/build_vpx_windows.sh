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

usage()
{
    echo "Download and build vpx for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

ARCH=""

while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done

if [ "$ARCH" != "i686" ] && [ "$ARCH" != "x86_64" ]; then
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

"$(dirname "$0")"/download/download_vpx.sh

if [ "${ARCH}" == "x86_64" ]; then
    ARCH_FLAGS="-fno-asynchronous-unwind-tables"
    VPX_ARCH="x86_64-win64-gcc"
elif [ "${ARCH}" == "i686" ]; then \
    ARCH_FLAGS=""
    VPX_ARCH="x86-win32-gcc"
else
    exit 1
fi

patch -Np1 < "$(dirname "$0")"/patches/vpx.patch

CFLAGS=${ARCH_FLAGS} CROSS="${ARCH}-w64-mingw32-" \
    ./configure --target="${VPX_ARCH}" \
        --prefix=/windows/ \
        --enable-shared \
        --disable-static \
        --enable-runtime-cpu-detect \
        --disable-examples \
        --disable-tools \
        --disable-docs \
        --disable-unit-tests

make -j $(nproc)
make install
