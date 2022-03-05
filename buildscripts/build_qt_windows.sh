#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

usage()
{
    echo "Download and build qt for the windows cross compiling environment"
    echo "Usage: $0 --arch {win64|win32}"
}

ARCH=""

while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done

if [[ "$ARCH" == "win64" ]]; then
    CROSS="x86_64-w64-mingw32-"
elif [[ "$ARCH" == "win32" ]]; then
    CROSS="i686-w64-mingw32-"
else
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

"$(dirname "$(realpath "$0")")/download/download_qt.sh"

OPENSSL_LIBS=$(pkg-config --libs openssl)
export OPENSSL_LIBS

./configure -prefix /windows/ \
    -release \
    -shared \
    -device-option CROSS_COMPILE=${CROSS} \
    -xplatform win32-g++ \
    -openssl \
    "$(pkg-config --cflags openssl)" \
    -opensource -confirm-license \
    -pch \
    -nomake examples \
    -nomake tools \
    -nomake tests \
    -skip 3d \
    -skip activeqt \
    -skip androidextras \
    -skip canvas3d \
    -skip charts \
    -skip connectivity \
    -skip datavis3d \
    -skip declarative \
    -skip doc \
    -skip gamepad \
    -skip graphicaleffects \
    -skip imageformats \
    -skip location \
    -skip macextras \
    -skip multimedia \
    -skip networkauth \
    -skip purchasing \
    -skip quickcontrols \
    -skip quickcontrols2 \
    -skip remoteobjects \
    -skip script \
    -skip scxml \
    -skip sensors \
    -skip serialbus \
    -skip serialport \
    -skip speech \
    -skip translations \
    -skip virtualkeyboard \
    -skip wayland \
    -skip webchannel \
    -skip webengine \
    -skip webglplugin \
    -skip websockets \
    -skip webview \
    -skip x11extras \
    -skip xmlpatterns \
    -no-dbus \
    -no-icu \
    -no-compile-examples \
    -qt-libjpeg \
    -qt-libpng \
    -qt-zlib \
    -qt-pcre \
    -opengl desktop

make -j $(nproc)
make install
