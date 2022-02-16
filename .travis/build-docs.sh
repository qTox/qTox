#!/bin/bash

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

# Obtain doxygen and its deps
sudo apt-get update -qq
sudo apt-get install doxygen graphviz

GIT_DESC=$(git describe --tags 2>/dev/null)
GIT_CHASH=$(git rev-parse HEAD)

# Append git version to doxygen version string
echo "PROJECT_NUMBER = \"Version: $GIT_DESC | Commit: $GIT_CHASH\"" >> "$DOXYGEN_CONFIG_FILE"

# Generate documentation
echo "Generating documentation..."
echo

doxygen "$DOXYGEN_CONFIG_FILE"
