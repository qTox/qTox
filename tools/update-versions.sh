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


# script to add versions to the files for osx and windows "packages"
# 
# it should be run by the `qTox-Mac-Deployer-ULTIMATE.sh`
#
# NOTE: it checkouts the files before appending a version to them!
#
# requires:
#  * git – tags in format `v0.0.0`
#  * GNU sed

# usage:
#
#   ./$script


set -eu -o pipefail


update_windows() {
    ( cd windows
        ./qtox-nsi-version.sh )
}

update_osx() {
    ( cd osx
        ./update-plist-version.sh )
}

main() {
    update_osx
    # osx cannot into proper sed
    if [[ ! "$OSTYPE" == "darwin"* ]]
    then
        update_windows
    fi
}
main
