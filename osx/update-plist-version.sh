#!/bin/bash
#
#    Copyright © 2016 The qTox Project Contributors
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


# script to append correct qTox version to `.plist` file from
# `git describe`
#
# NOTE: it checkouts the files before appending a version to them!
#
# requires:
#  * correctly formatted `*.plist file(s) in working dir
#  * git – tags in format `v0.0.0`
#  * GNU sed

# usage:
#
#   ./$script

set -eu -o pipefail

# uses `get_version()`
source "../tools/lib/git.source"


# append version to .plist file(s) after the right line
append_version() {
    local after_line='		<key>CFBundleVersion'
    local append="		<string>$(get_version)<\/string>"

    for plist in *.plist
    do
        git checkout "$plist"
        sed -i "/$after_line/a\\$append" "$plist"
    done
}
append_version
