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


# script to change qTox version in `info.plist` file to the supplied one
#
# NOTE: it checkouts the files before appending a version to them!
#
# requires:
#  * correctly formatted `info.plist file in working dir
#  * GNU sed

# usage:
#
#   ./$script $version
#
# $version has to be composed of at least one number/dot

set -eu -o pipefail

# update version in `info.plist` file to supplied one after the right lines
update_version() {
    local vars=(
        '	<key>CFBundleShortVersionString</key>'
        '	<key>CFBundleVersion</key>'
    )

    for v in "${vars[@]}"
    do
        sed -i -r "\\R$v\$R,+1 s,(<string>)[0-9\\.]+(</string>)$,\\1$@\\2," \
            "./info.plist"
    done
}

# exit if supplied arg is not a version
is_version() {
    if [[ ! $@ =~ [0-9\\.]+ ]]
    then
        echo "Not a version: $@"
        exit 1
    fi
}

main() {
    is_version "$@"

    update_version "$@"
}
main "$@"
