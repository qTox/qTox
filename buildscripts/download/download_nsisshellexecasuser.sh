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

NSISSHELLEXECASUSER_HASH=79bdd3e54a9ba9c30af85557b475d2322286f8726687f2e23afa772aac6049ab

source "$(dirname "$(realpath "$0")")/common.sh"

# Backup: https://web.archive.org/web/20220314233613/https://nsis.sourceforge.io/mediawiki/images/1/1d/ShellExecAsUserUnicodeUpdate.zip
download_file https://nsis.sourceforge.io/mediawiki/images/1/1d/ShellExecAsUserUnicodeUpdate.zip

if ! check_sha256 "$NSISSHELLEXECASUSER_HASH" ShellExecAsUserUnicodeUpdate.zip; then
    exit 1
fi

unzip ShellExecAsUserUnicodeUpdate.zip
rm ShellExecAsUserUnicodeUpdate.zip
