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

TOXCORE_VERSION=0.2.17
TOXCORE_HASH=d9cdf5d690fe8e217dcaba5b988fb943bbe7bda919a2fde6d9506df047e5f4cb

source "$(dirname "$(realpath "$0")")/common.sh"

download_verify_extract_tarball \
    https://github.com/TokTok/c-toxcore/archive/v$TOXCORE_VERSION.tar.gz \
    "$TOXCORE_HASH"
