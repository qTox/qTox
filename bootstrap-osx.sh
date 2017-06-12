#!/usr/bin/env bash

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
