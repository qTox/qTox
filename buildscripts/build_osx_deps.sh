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

SCRIPT_DIR=$(dirname $(realpath "$0"))

install_dep()
{
    mkdir -p _build-dep
    pushd _build-dep
    "$SCRIPT_DIR"/$1 --arch macos
    popd
    rm -rf _build-dep
}

install_dep build_openssl_cross.sh
install_dep build_qrencode_cross.sh
install_dep build_libexif_cross.sh
install_dep build_sodium_cross.sh
install_dep build_openal_cross.sh
install_dep build_vpx_cross.sh
install_dep build_opus_cross.sh
install_dep build_ffmpeg_cross.sh
install_dep build_msgpack_c_cross.sh
install_dep build_toxcore_cross.sh
install_dep build_sqlcipher_cross.sh
# install_dep build_qt_cross.sh
