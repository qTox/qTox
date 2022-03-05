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

GDB_VERSION="11.1"
GDB_HASH="cccfcc407b20d343fb320d4a9a2110776dd3165118ffd41f4b1b162340333f94"

source "$(dirname "$(realpath "$0")")/common.sh"

download_verify_extract_tarball \
    "http://ftp.gnu.org/gnu/gdb/gdb-${GDB_VERSION}.tar.xz" \
    ${GDB_HASH}
