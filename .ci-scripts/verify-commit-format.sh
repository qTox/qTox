#!/bin/bash
#
#    Copyright © 2016-2019 by The qTox Project Contributors
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
#

# Fail out on error
set -eu -o pipefail

# Verify commit messages
readarray -t COMMITS <<<$(curl -s ${GITHUB_CONTEXT} | jq -r '.[0,-1].sha')
COMMIT_RANGE="${COMMITS[1]}~1..${COMMITS[0]}"
bash ./verify-commit-messages.sh "$COMMIT_RANGE"
