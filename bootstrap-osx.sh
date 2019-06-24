#!/usr/bin/env bash

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


# This script's purpose is to ease compiling qTox for users.
#
# NO AUTOMATED BUILDS SHOULD DEPEND ON IT.
#
# This script is and will be a subject to breaking changes, and at no time one
# should expect it to work - it's something that you could try to use but
# don't expect that it will work for sure.
#
# If script doesn't work, you should use instructions provided in INSTALL.md
# before reporting issues like “qTox doesn't compile”.
#
# With that being said, reporting that this script doesn't work would be nice.
#
# If you are contributing code to qTox that change its dependencies / the way
# it's being build, please keep in mind that changing just bootstrap.sh
# *IS NOT* and will not be sufficient - you should update INSTALL.md first.

set -eu -o pipefail


# copy libs to given destination
copy_libs() {
    local dest="$@"
    local libs=(
        /usr/local/lib/libsodium*
        /usr/local/lib/libvpx*
        /usr/local/lib/libopus*
        /usr/local/lib/libav*
        /usr/local/lib/libswscale*
        /usr/local/lib/libqrencode*
        /usr/local/lib/libsqlcipher*
    )
    echo Copying libraries…
    for lib in "${libs[@]}"
    do
        cp -v "$lib" "$dest"
    done
}

# copy includes to given destination
copy_includes() {
    local dest="$@"
    local includes=(
        /usr/local/include/vpx*
        /usr/local/include/sodium*
        /usr/local/include/qrencode*
        /usr/local/include/libav*
        /usr/local/include/libswscale*
        /usr/local/include/sqlcipher*
    )
    echo Copying include files…
    for include in "${includes[@]}"
    do
        cp -v -r "$include" "$dest"
    done
}

main() {
    local libs_dir="libs/lib"
    local inc_dir="libs/include"
    echo Creating directories…
    mkdir -v -p "$libs_dir" "$inc_dir"
    copy_libs "$libs_dir"
    copy_includes "$inc_dir"
    echo Done.
}
main
