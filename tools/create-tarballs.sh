#!/bin/bash

#   Copyright © 2017-2019 by The qTox Project Contributors
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

# Create `lzip` and `gzip` archives and make detached GPG signatures for them.
#
# When tag name is supplied, it's used to create archives. If there is no tag
# name supplied, latest tag is used.

# Requires:
#   * GPG
#   * git
#   * gzip
#   * lzip

# usage:
#   ./$script [$tag_name]


# Fail as soon as error appears
set -eu -o pipefail


archive() {
    git archive --format=tar --prefix=qTox/ "${@%%.tar.*}" \
    | "${@##*.tar.}"ip --best \
    > "$@"
    echo "$@ archive has been created."
}

sign_archive() {
    gpg \
        --armor \
        --detach-sign \
        "$@"
    echo "$@.asc signature has been created."
}

create_and_sign() {
    local archives=("$@".tar.{l,g}z)
    for a in "${archives[@]}"
    do
        archive "$a"
        sign_archive "$a"
    done
}

get_tag() {
    local tname="$@"
    if [[ -n "$tname" ]]
    then
        echo "$tname"
    else
        git describe --abbrev=0
    fi
}

main() {
    create_and_sign "$(get_tag $@)"
}
main "$@"
