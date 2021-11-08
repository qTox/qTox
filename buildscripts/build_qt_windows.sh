#!/bin/bash

#    Copyright © 2021 by The qTox Project Contributors
#
#    This program is libre software: you can redistribute it and/or modify
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

set -euo pipefail

usage()
{
    echo "Download and build qt for the windows cross compiling environment"
    echo "Usage: $0 --arch {x86_64|i686}"
}

ARCH=""

while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
done

if [ "$ARCH" != "i686" ] && [ "$ARCH" != "x86_64" ]; then
    echo "Unexpected arch $ARCH"
    usage
    exit 1
fi

"$(dirname "$0")"/download/download_qt.sh

OPENSSL_LIBS=$(pkg-config --libs openssl)
export OPENSSL_LIBS

./configure -prefix /windows/ \
    -release \
    -shared \
    -device-option CROSS_COMPILE=${ARCH}-w64-mingw32- \
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
