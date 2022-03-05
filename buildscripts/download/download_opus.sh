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

OPUS_VERSION=1.3.1
# https://archive.mozilla.org/pub/opus/SHA256SUMS.txt
OPUS_HASH=65b58e1e25b2a114157014736a3d9dfeaad8d41be1c8179866f144a2fb44ff9d

source "$(dirname "$(realpath "$0")")/common.sh"

download_verify_extract_tarball \
    "https://archive.mozilla.org/pub/opus/opus-${OPUS_VERSION}.tar.gz" \
    "${OPUS_HASH}"
