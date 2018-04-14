#!/usr/bin/env bash

# MIT License
#
# Copyright © 2018 by The qTox Project Contributors
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
readonly APT_FLAGS="-y --no-install-recommends"
# use multiple cores when building
export MAKEFLAGS="-j$(nproc)"

# Get packages
apt-get update
apt-get install $APT_FLAGS sudo ca-certificates wget fuse xxd git g++ patchelf

# get version
cd "$QTOX_SRC_DIR"
export VERSION=$(git rev-parse --short HEAD) # linuxdeployqt uses this for naming the file


# create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# copy qtox source
cp -r "$QTOX_SRC_DIR" "$QTOX_BUILD_DIR"
cd "$QTOX_BUILD_DIR"

# reuse for our purposes, pass flags to automatically install packages
./simple_make.sh "$APT_FLAGS"

# build dir of simple_make
cd _build

make DESTDIR="$QTOX_APP_DIR" install ; find "$QTOX_APP_DIR"

LDQT_HASH="9c90a882ac744b5f704598e9588450ddfe487c67" # is master as of 2018-04-25
# build linuxdeployqt
git clone https://github.com/probonopd/linuxdeployqt.git "$LDQT_BUILD_DIR"
cd "$LDQT_BUILD_DIR"
git checkout "$LDQT_HASH"
qmake
make
make install

AITOOL_HASH="5d93115f279d94a4d23dfd64fb8ccd109e98f039" # is master as of 2018-04-25
# build appimagetool
git clone -b appimagetool/master --single-branch --recursive https://github.com/AppImage/AppImageKit "$AITOOL_BUILD_DIR"
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

cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON -DAPPIMAGEKIT_PACKAGE_DEBS=ON
make
make install

# build qtox AppImage
cd "$OUTPUT_DIR"
unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH;

readonly QTOX_DESKTOP_FILE="$QTOX_APP_DIR"/usr/local/share/applications/*.desktop

eval "$LDQT_BIN $QTOX_DESKTOP_FILE -bundle-non-qt-libs"
eval "$LDQT_BIN $QTOX_DESKTOP_FILE -appimage"

# Chmod since everything is root:root
chmod 755 -R "$OUTPUT_DIR"
