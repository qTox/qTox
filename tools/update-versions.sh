#!/bin/bash
#
#    Copyright © 2016-2018 The qTox Project Contributors
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


# script to change versions in the files for osx and windows "packages"
# 
# it should be run before releasing a new version
#
# NOTE: it checkouts the files before appending a version to them!
#
# requires:
#  * GNU sed

# usage:
#
#   ./$script $version
#
# $version has to be composed of at least one number/dot


set -eu -o pipefail

readonly SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
readonly BASE_DIR="$SCRIPT_DIR/../"
readonly VERSION_PATTERN="[0-9]+\.[0-9]+\.[0-9]+"

update_windows() {
    ( cd "$BASE_DIR/windows"
        ./qtox-nsi-version.sh "$@" )
}

update_osx() {
    ( cd "$BASE_DIR/osx"
        ./update-plist-version.sh "$@" )
}

update_readme() {
    cd "$BASE_DIR"
    sed -ri "s|(github.com/qTox/qTox/releases/download/v)$VERSION_PATTERN|\1$@|g" README.md
    # for flatpak and AppImage
    sed -ri "s|(github.com/qTox/qTox/releases/download/v$VERSION_PATTERN/qTox-v)$VERSION_PATTERN|\1$@|g" README.md
}

# exit if supplied arg is not a version
is_version() {
    if [[ ! $@ =~ $VERSION_PATTERN ]]
    then
        echo "Not a version: $@"
        echo "Must match: $VERSION_PATTERN"
        exit 1
    fi
}

main() {
    is_version "$@"

    # osx cannot into proper sed
    if [[ ! "$OSTYPE" == "darwin"* ]]
    then
        update_osx "$@"
        update_windows "$@"
        update_readme "$@"
    else
        # TODO: actually check whether there is a GNU sed on osx
        echo "OSX's sed not supported. Get a proper one."
    fi
}
main "$@"
