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

NSISSHELLEXECASUSER_HASH=8fc19829e144716a422b15a85e718e1816fe561de379b2b5ae87ef9017490799

source "$(dirname "$0")/common.sh"

download_file http://nsis.sourceforge.net/mediawiki/images/c/c7/ShellExecAsUser.zip

if ! check_sha256 "$NSISSHELLEXECASUSER_HASH" ShellExecAsUser.zip; then
    exit 1
fi

unzip ShellExecAsUser.zip
rm ShellExecAsUser.zip
