#!/bin/bash

#    Copyright © 2016-2019 The qTox Project Contributors
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

# Scripts for generating gitstats in CI
#
# Downloads current git repo, and builds its stats.

# usage:
#   ./$script

# Fail as soon as an error appears
set -eu -o pipefail


make_stats() {
    gitstats \
        -c authors_top=1000 \
        -c max_authors=100000 \
        . \
        "$GITSTATS_DIR"
}

# check if at least something has been generated
verify_exists() {
    [[ -e "$GITSTATS_DIR/index.html" ]]
}


main() {
    make_stats
    verify_exists
}
main
