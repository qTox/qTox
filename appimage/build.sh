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
# sqlcipher build directory
readonly SQLCIPHER_BUILD_DIR="$BUILD_DIR"/sqlcipher
# ldqt binary
readonly LDQT_BIN="/usr/lib/x86_64-linux-gnu/qt5/bin/linuxdeployqt"
readonly APT_FLAGS="-y --no-install-recommends"
# use multiple cores when building
export MAKEFLAGS="-j$(nproc)"

# Get packages
apt-get update
apt-get install $APT_FLAGS sudo ca-certificates wget build-essential fuse xxd \
git g++ patchelf tclsh libssl-dev

# get version
cd "$QTOX_SRC_DIR"
# linuxdeployqt uses this for naming the file
export VERSION=$(git rev-parse --short HEAD)


# create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# we need a custom built sqlcipher version because of a Debian bug
# https://bugs.debian.org/850421
git clone https://github.com/sqlcipher/sqlcipher.git "$SQLCIPHER_BUILD_DIR"
cd "$SQLCIPHER_BUILD_DIR"
git checkout tags/v3.4.2
./configure --enable-tempstore=yes CFLAGS="-DSQLITE_HAS_CODEC" \
LDFLAGS="-lcrypto"

make
make install

# copy qtox source
cp -r "$QTOX_SRC_DIR" "$QTOX_BUILD_DIR"
cd "$QTOX_BUILD_DIR"

# ensure this directory is empty
rm -rf ./_build

# reuse for our purposes, pass flags to automatically install packages
# APT_FLAGS for automatic install
# True to not install sqlcipher
./simple_make.sh "$APT_FLAGS" True

# build dir of simple_make
cd _build

make DESTDIR="$QTOX_APP_DIR" install ; find "$QTOX_APP_DIR"

# is master as of 2018-04-25
LDQT_HASH="9c90a882ac744b5f704598e9588450ddfe487c67"
# build linuxdeployqt
git clone https://github.com/probonopd/linuxdeployqt.git "$LDQT_BUILD_DIR"
cd "$LDQT_BUILD_DIR"
git checkout "$LDQT_HASH"
qmake
make
make install

# is master as of 2018-04-25
AITOOL_HASH="5d93115f279d94a4d23dfd64fb8ccd109e98f039"
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
-DAPPIMAGEKIT_PACKAGE_DEBS=ON -DUPDATE_CHECK=ON

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
