#!/usr/bin/env bash

# MIT License
#
# Copyright © 2019 by The qTox Project Contributors
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# Fail out on error
set -exuo pipefail

while (( $# > 0 )); do
    case $1 in
        --src-dir) QTOX_SRC_DIR=$2; shift 2 ;;
        *) shift ;;
    esac
done

# directory paths
BUILD_DIR=$(realpath .)
readonly BUILD_DIR
QTOX_APP_DIR="$BUILD_DIR"/appdir
readonly QTOX_APP_DIR
readonly LOCAL_LIB_DIR="$QTOX_APP_DIR"/local/lib
# ldqt binary
readonly LDQT_BIN="/usr/lib/qt5/bin/linuxdeployqt"

# update information to be embeded in AppImage
if [ "cron" == "${TRAVIS_EVENT_TYPE:-}" ]
then
    # update information for nightly version
    readonly NIGHTLY_REPO_SLUG=$(echo "$CIRP_GITHUB_REPO_SLUG" | tr "/" "|")
    readonly UPDATE_INFO="gh-releases-zsync|$NIGHTLY_REPO_SLUG|ci-master-latest|qTox-*-x86_64.AppImage.zsync"
else
    # update information for stable version
    readonly UPDATE_INFO="gh-releases-zsync|qTox|qTox|latest|qTox-*.x86_64.AppImage.zsync"
fi

export VERSION=$(git -C "${QTOX_SRC_DIR}" rev-parse --short HEAD)

echo $QTOX_APP_DIR
cmake "${QTOX_SRC_DIR}" -DDESKTOP_NOTIFICATIONS=ON -DUPDATE_CHECK=ON -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
rm -fr appdir
make DESTDIR=appdir install

unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH;

readonly QTOX_DESKTOP_FILE="$QTOX_APP_DIR"/usr/local/share/applications/*.desktop
export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib/x86_64-linux-gnu/

eval "$LDQT_BIN $QTOX_DESKTOP_FILE -bundle-non-qt-libs -extra-plugins=libsnore-qt5"

# Move the required files to the correct directory
mv "$QTOX_APP_DIR"/usr/* "$QTOX_APP_DIR/"
rm -rf "$QTOX_APP_DIR/usr"

# Warning: This is hard coded to debain:stretch.
libs=(
# copy OpenSSL libs to AppImage
/usr/lib/x86_64-linux-gnu/libssl.so
/usr/lib/x86_64-linux-gnu/libcrypt.so
/usr/lib/x86_64-linux-gnu/libcrypto.so
# Also bundle libjack.so* without which the AppImage does not work in Fedora Workstation
/usr/lib/x86_64-linux-gnu/libjack.so.0
# And libglib needed by Red Hat and derivatives to work with our old gnutls
/usr/lib/x86_64-linux-gnu/libglib-2.0.so.0
/usr/lib/x86_64-linux-gnu/libgobject-2.0.so.0
/usr/lib/x86_64-linux-gnu/libgio-2.0.so.0
/usr/lib/x86_64-linux-gnu/libpango-1.0.so.0
/usr/lib/x86_64-linux-gnu/libpangoft2-1.0.so.0
)

for lib in "${libs[@]}"; do
    lib_file_name=$(basename "$lib")
    cp -P $(echo "$lib"*) "$LOCAL_LIB_DIR"
    patchelf --set-rpath '$ORIGIN' "$LOCAL_LIB_DIR/$lib_file_name"
done

# this is important, aitool automatically uses the same filename in .zsync meta file.
# if this name does not match with the one we upload, the update always fails.
if [ -n "${TRAVIS_TAG-}" ]
then
    VERSION_NAME="${TRAVIS_TAG}"
elif [ -n "${TRAVIS_COMMIT-}" ]
then
    VERSION_NAME="${TRAVIS_COMMIT}"
else
    VERSION_NAME="${VERSION}"
fi

appimagetool -u "$UPDATE_INFO" $QTOX_APP_DIR qTox-$VERSION_NAME.x86_64.AppImage
