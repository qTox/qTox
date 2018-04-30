#!/bin/bash
#
#    Copyright © 2017-2018 The qTox Project Contributors
#
#    This program is free software: you can redistribute it and/or modify
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

# Format all C++ codebase tracked by git using `clang-format`.

# Requires:
#   * git
#   * clang-format

# usage:
#   ./$script


# Fail as soon as error appears
set -eu -o pipefail


readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly BASE_DIR="$SCRIPT_DIR/../"

format() {
    cd "$BASE_DIR"
    [[ -f .clang-format ]] # make sure that it exists
    # NOTE: some earlier than 3.8 versions of clang-format are broken
    # and will not work correctly
    clang-format -i -style=file $(git ls-files *.cpp *.h)
}
format
