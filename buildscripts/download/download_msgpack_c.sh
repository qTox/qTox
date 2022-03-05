#!/bin/bash

#    Copyright © 2022 by The qTox Project Contributors
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

MSGPACK_VERSION=c-4.0.0
MSGPACK_HASH=656ebe4566845e7bda9c097b625ba59ac72ddfd45df6017172d46d9ac7365aa3

source "$(dirname "$(realpath "$0")")/common.sh"

download_verify_extract_tarball \
    "https://github.com/msgpack/msgpack-c/archive/${MSGPACK_VERSION}.tar.gz" \
    "${MSGPACK_HASH}"
