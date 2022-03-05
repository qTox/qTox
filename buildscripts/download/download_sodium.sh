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

SODIUM_VERSION=1.0.18
SODIUM_HASH=6f504490b342a4f8a4c4a02fc9b866cbef8622d5df4e5452b46be121e46636c1

source "$(dirname "$(realpath "$0")")/common.sh"

download_verify_extract_tarball \
    "https://download.libsodium.org/libsodium/releases/libsodium-${SODIUM_VERSION}.tar.gz" \
    "${SODIUM_HASH}"
