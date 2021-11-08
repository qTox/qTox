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

usage()
{
    echo "Download and build gmp for windows"
    echo "Usage: $0 --arch {x86_64|i686}"
}

set -euo pipefail

while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done

if [ "${ARCH-x}" != "i686" ] && [ "${ARCH-x}" != "x86_64" ]; then
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

set -euo pipefail

"$(dirname $0)"/download/download_gmp.sh

# https://gmplib.org/list-archives/gmp-discuss/2020-July/006519.html
CC_FOR_BUILD=gcc CFLAGS="-O2 -g0" ./configure --host="$ARCH-w64-mingw32" \
                                --prefix="/windows" \
                                --enable-static \
                                --disable-shared
make -j $(nproc)
make install
