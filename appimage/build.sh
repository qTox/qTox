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

# directory paths
readonly QTOX_SRC_DIR="/qtox"
readonly OUTPUT_DIR="/output"
readonly BUILD_DIR="/build"
readonly QTOX_BUILD_DIR="$BUILD_DIR"/qtox
readonly QTOX_APP_DIR="$BUILD_DIR"/appdir
# "linuxdeployqt" is to long, shortened to ldqt
readonly LDQT_BUILD_DIR="$BUILD_DIR"/ldqt
# "appimagetool" becomes aitool
readonly AITOOL_BUILD_DIR="$BUILD_DIR"/aitool
# ldqt binary
readonly LDQT_BIN="/usr/lib/x86_64-linux-gnu/qt5/bin/linuxdeployqt"
# aitool binary
readonly AITOOL_BIN="/usr/local/bin/appimagetool"
readonly APPRUN_BIN="/usr/local/bin/AppRun"
readonly APT_FLAGS="-y --no-install-recommends"
# snorenotify source
readonly SNORE_GIT="https://github.com/KDE/snorenotify"
# snorenotify build directory
readonly SNORE_BUILD_DIR="$BUILD_DIR"/snorenotify

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

# use multiple cores when building
export MAKEFLAGS="-j$(nproc)"

# Get packages
apt-get update
apt-get install $APT_FLAGS sudo ca-certificates wget build-essential fuse xxd \
git patchelf tclsh libssl-dev cmake extra-cmake-modules build-essential \
check checkinstall libavdevice-dev libexif-dev libgdk-pixbuf2.0-dev \
libgtk2.0-dev libopenal-dev libopus-dev libqrencode-dev libqt5opengl5-dev \
libqt5svg5-dev libsodium-dev libtool libvpx-dev libxss-dev \
qt5-default qttools5-dev qttools5-dev-tools qtdeclarative5-dev \
fcitx-frontend-qt5 uim-qt5 libsqlcipher-dev

# get version
cd "$QTOX_SRC_DIR"
# linuxdeployqt uses this for naming the file
export VERSION=$(git rev-parse --short HEAD)


# create build directory
mkdir -p "$BUILD_DIR"

# install snorenotify because it's not packaged
cd "$BUILD_DIR"
git clone "$SNORE_GIT" "$SNORE_BUILD_DIR"
cd "$SNORE_BUILD_DIR"
git checkout tags/v0.7.0
# HACK: Kids, don't do this at your home system
cmake -DCMAKE_INSTALL_PREFIX=/usr/
make
make install

cd "$BUILD_DIR"

# copy qtox source
cp -r "$QTOX_SRC_DIR" "$QTOX_BUILD_DIR"
cd "$QTOX_BUILD_DIR"

./bootstrap.sh

# ensure this directory is empty
rm -rf ./_build
mkdir -p ./_build

# build dir of simple_make
cd _build

# need to build with -DDESKTOP_NOTIFICATIONS=True for snorenotify
cmake -DDESKTOP_NOTIFICATIONS=True \
      -DUPDATE_CHECK=True \
      -DSTRICT_OPTIONS=True \
      ../

make

make DESTDIR="$QTOX_APP_DIR" install ; find "$QTOX_APP_DIR"

# is release #6 as of 2019-10-23
LDQT_HASH="37631e5640d8f7c31182fa72b31266bbdf6939fc"
# build linuxdeployqt
git clone https://github.com/probonopd/linuxdeployqt.git "$LDQT_BUILD_DIR"
cd "$LDQT_BUILD_DIR"
git checkout "$LDQT_HASH"
qmake
make
make install

# is release #12 as of 2019-10-23
AITOOL_HASH="effcebc1d81c5e174a48b870cb420f490fb5fb4d"
# build appimagetool
git clone -b master --single-branch --recursive \
https://github.com/AppImage/AppImageKit "$AITOOL_BUILD_DIR"

cd "$AITOOL_BUILD_DIR"
git checkout "$AITOOL_HASH"
bash -ex install-build-deps.sh
# Fetch git submodules
git submodule update --init --recursive

# Build AppImage
mkdir build
cd build

# make sure that deps in separate install tree are found
export PKG_CONFIG_PATH=/deps/lib/pkgconfig/

cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON \
-DAPPIMAGEKIT_PACKAGE_DEBS=ON

make
make install

# build qtox AppImage
cd "$OUTPUT_DIR"
unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH;

readonly QTOX_DESKTOP_FILE="$QTOX_APP_DIR"/usr/local/share/applications/*.desktop

eval "$LDQT_BIN $QTOX_DESKTOP_FILE -bundle-non-qt-libs -extra-plugins=libsnore-qt5"

# Move the required files to the correct directory
mv "$QTOX_APP_DIR"/usr/* "$QTOX_APP_DIR/"
rm -rf "$QTOX_APP_DIR/usr"

# copy OpenSSL libs to AppImage
# Warning: This is hard coded to debain:stretch.
cp /usr/lib/x86_64-linux-gnu/libssl.so* "$QTOX_APP_DIR/local/lib/"
cp /usr/lib/x86_64-linux-gnu/libcrypt.so* "$QTOX_APP_DIR/local/lib/"
cp /usr/lib/x86_64-linux-gnu/libcrypto.so* "$QTOX_APP_DIR/local/lib"
# Also bundle libjack.so* without which the AppImage does not work in Fedora Workstation
cp /usr/lib/x86_64-linux-gnu/libjack.so* "$QTOX_APP_DIR/local/lib"

# this is important , aitool automatically uses the same filename in .zsync meta file.
# if this name does not match with the one we upload , the update always fails.
if [ -n "$TRAVIS_TAG" ]
then
    eval "$AITOOL_BIN -u \"$UPDATE_INFO\" $QTOX_APP_DIR qTox-$TRAVIS_TAG.x86_64.AppImage"
else
    eval "$AITOOL_BIN -u \"$UPDATE_INFO\" $QTOX_APP_DIR qTox-$TRAVIS_COMMIT-x86_64.AppImage"
fi

# Chmod since everything is root:root
chmod 755 -R "$OUTPUT_DIR"
