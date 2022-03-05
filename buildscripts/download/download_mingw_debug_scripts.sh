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

MINGW_W64_DEBUG_SCRIPTS_VERSION="c6ae689137844d1a6fd9c1b9a071d3f82a44c593"
MINGW_W64_DEBUG_SCRIPTS_HASH="5916bf9e6691d4f7f1c233c8c3a2b9f3b1d1ad0f58ab951381da5ec856f5d021"

source "$(dirname "$(realpath "$0")")/common.sh"

download_verify_extract_tarball \
    "https://github.com/nurupo/mingw-w64-debug-scripts/archive/${MINGW_W64_DEBUG_SCRIPTS_VERSION}.tar.gz" \
    ${MINGW_W64_DEBUG_SCRIPTS_HASH}
