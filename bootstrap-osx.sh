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

echo Creating directories…
mkdir -p libs/lib
mkdir -p libs/include
echo Copying libraries…
cp /usr/local/lib/libsodium* libs/lib
cp /usr/local/lib/libvpx* libs/lib
cp /usr/local/lib/libopus* libs/lib
cp /usr/local/lib/libav* libs/lib
cp /usr/local/lib/libswscale* libs/lib
cp /usr/local/lib/libqrencode* libs/lib
cp /usr/local/lib/libsqlcipher* libs/lib
echo Copying include files...
cp -r /usr/local/include/vpx* libs/include
cp -r /usr/local/include/sodium* libs/include
cp -r /usr/local/include/qrencode* libs/include
cp -r /usr/local/include/libav* libs/include
cp -r /usr/local/include/libswscale* libs/include
cp -r /usr/local/include/sqlcipher* libs/include
echo Done.

