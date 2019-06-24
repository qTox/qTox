#!/bin/bash

#   Copyright © 2016-2019 by The qTox Project Contributors
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


# script to change targeted toxcore version
# 
# it should be run before releasing a new version

# usage:
#
#   ./$script $version
#
# $version has to be composed of at least one number/dot


set -eu -o pipefail

readonly SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
readonly BASE_DIR="$SCRIPT_DIR/../"
readonly VERSION_PATTERN="[0-9]+\.[0-9]+\.[0-9]+"

update_install() {
    perl -i -0pe "s|(git clone https://github.com/toktok/c-toxcore.*?)$VERSION_PATTERN|\${1}$@|gms" INSTALL.md
    perl -i -0pe "s|(git clone https://github.com/toktok/c-toxcore.*?)$VERSION_PATTERN|\${1}$@|gms" INSTALL.fa.md
}

update_bootstrap() {
    perl -i -0pe "s|(TOXCORE_VERSION=\"v)$VERSION_PATTERN|\${1}$@|gms" bootstrap.sh
}

update_osx() {
    cd osx
    perl -i -0pe "s|$VERSION_PATTERN( --depth=1 https://github.com/toktok/c-toxcore)|$@\${1}|gms" qTox-Mac-Deployer-ULTIMATE.sh
    cd ..
}

update_flatpak() {
    cd flatpak
    latest_tag_ref=$(git ls-remote --tags https://github.com/toktok/c-toxcore | tail -n1)
    ref_array=($latest_tag_ref)
    commit_hash=${ref_array[0]}
    perl -i -0pe "s|(https://github.com/toktok/c-toxcore.*?)$VERSION_PATTERN(.*?)[a-f0-9]{40}|\${1}$@\${2}$commit_hash|gms" io.github.qtox.qTox.json
    cd ..
}

update_travis() {
    cd .travis
    perl -i -0pe "s|$VERSION_PATTERN( --depth=1 https://github.com/toktok/c-toxcore)|$@\${1}|gms" build-ubuntu-14-04.sh
    cd ..
}

update_docker() {
    cd docker
    perl -i -0pe "s|(https://github.com/toktok/c-toxcore.*?)$VERSION_PATTERN|\${1}$@|gms" Dockerfile.debian
    perl -i -0pe "s|(https://github.com/toktok/c-toxcore.*?)$VERSION_PATTERN|\${1}$@|gms" Dockerfile.ubuntu
    cd ..
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
    update_install "$@"
    update_bootstrap "$@"
    update_osx "$@"
    update_flatpak "$@"
    update_travis "$@"
    update_docker "$@"
}
main "$@"
