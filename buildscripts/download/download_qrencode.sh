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

QRENCODE_VERSION=4.1.1
QRENCODE_HASH=e455d9732f8041cf5b9c388e345a641fd15707860f928e94507b1961256a6923

source "$(dirname "$(realpath "$0")")/common.sh"

download_verify_extract_tarball \
    "https://fukuchi.org/works/qrencode/qrencode-${QRENCODE_VERSION}.tar.bz2" \
    "${QRENCODE_HASH}"
