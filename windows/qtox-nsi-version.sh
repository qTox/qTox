#!/bin/bash

#   Copyright © 2016-2017 Zetok Zalbavar <zetok@openmailbox.org>
#   Copyright © 2019 by The qTox Project Contributors
#
#   This file is part of qTox, a Qt-based graphical interface for Tox.
#   qTox is libre software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   qTox is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with qTox.  If not, see <http://www.gnu.org/licenses/>


# script to change qTox version in `.nsi` files to supplied one
#
# requires:
#  * files `qtox.nsi` and `qtox64.nsi` in working dir
#  * GNU sed

# usage:
#
#   ./$script $version
#
# $version has to be composed of at least one number/dot

set -eu -o pipefail


# change version in .nsi files in the right line
change_version() {
    for nsi in *.nsi
    do
        sed -i -r "/DisplayVersion/ s/\"[0-9\\.]+\"$/\"$@\"/" "$nsi"
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

    change_version "$@"
}
main "$@"
