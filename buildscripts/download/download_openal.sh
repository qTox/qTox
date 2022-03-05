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

OPENAL_VERSION=b80570bed017de60b67c6452264c634085c3b148
OPENAL_HASH=e9f6d37672e085d440ef8baeebb7d62fec1d152094c162e5edb33b191462bd78

source "$(dirname "$(realpath "$0")")/common.sh"

## We can stop using the fork once OpenAL-Soft gets loopback capture implemented:
## https://github.com/kcat/openal-soft/pull/421
download_verify_extract_tarball \
    "https://github.com/irungentoo/openal-soft-tox/archive/${OPENAL_VERSION}.tar.gz" \
    "${OPENAL_HASH}"
