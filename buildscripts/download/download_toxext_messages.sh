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

TOXEXT_MESSAGES_VERSION=0.0.3
TOXEXT_MESSAGES_HASH=e7a9a199a3257382a85a8e555b6c8c540b652a11ca9a471b9da2a25a660dfdc3

source "$(dirname "$(realpath "$0")")/common.sh"

download_verify_extract_tarball \
    https://github.com/toxext/tox_extension_messages/archive/v$TOXEXT_MESSAGES_VERSION.tar.gz \
    "$TOXEXT_MESSAGES_HASH"
