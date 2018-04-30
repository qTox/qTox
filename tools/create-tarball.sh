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

# Create a `lzip` archive and make a detached GPG signature for it.
#
# When tag name is supplied, it's used to create archive. If there is no tag
# name supplied, latest tag is used.

# Requires:
#   * GPG
#   * git
#   * lzip

# usage:
#   ./$script [$tag_name]


# Fail as soon as error appears
set -eu -o pipefail


archive_from_tag() {
    git archive --format=tar "$@" \
    | lzip --best \
    > "$@".tar.lz
    echo "$@.tar.lz archive has been created."
}

sign_archive() {
    gpg \
        --armor \
        --detach-sign \
        "$@".tar.lz
    echo "$@.tar.lz.asc signature has been created."
}

create_and_sign() {
    archive_from_tag "$@"
    sign_archive "$@"
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
