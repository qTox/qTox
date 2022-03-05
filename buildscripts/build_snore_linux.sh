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

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"

usage ()
{
    echo "Build/install snore for linux"
    echo "Usage: $0 [--system-install]"
    echo "--system-install: Install to /usr instead of /usr/local"
}

SYSTEM_INSTALL=0
while (( $# > 0 )); do
    case $1 in
    --system-install) SYSTEM_INSTALL=1; shift ;;
    --help|-h) usage; exit 1 ;;
    *) echo "Unexpected argument $1"; usage; exit 1 ;;
    esac
done

"${SCRIPT_DIR}/download/download_snore.sh"

# Snore needs to be installed into /usr for linuxdeployqt to find it
if [ $SYSTEM_INSTALL -eq 1 ]; then
    INSTALL_PREFIX_ARGS="-DCMAKE_INSTALL_PREFIX=/usr"
else
    INSTALL_PREFIX_ARGS=""
fi

patch -Np1 < "${SCRIPT_DIR}/patches/snore.patch"

cmake -DCMAKE_BUILD_TYPE=Release $INSTALL_PREFIX_ARGS \
    -DBUILD_daemon=OFF \
    -DBUILD_settings=OFF \
    -DBUILD_snoresend=OFF

make -j $(nproc)
make install
