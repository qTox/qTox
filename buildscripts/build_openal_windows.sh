#!/bin/bash

set -euo pipefail

usage()
{
    echo "Download and build openal for the windows cross compiling environment"
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

"$(dirname "$0")"/download/download_openal.sh

patch -p1 < "$(dirname "$0")"/patches/openal-cmake-3-11.patch

export CFLAGS="-fPIC"
cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
    -DCMAKE_BUILD_TYPE=Release \
    -DALSOFT_UTILS=OFF \
    -DALSOFT_EXAMPLES=OFF \
    -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
    -DDSOUND_INCLUDE_DIR=/usr/${ARCH}-w64-mingw32/include \
    -DDSOUND_LIBRARY=/usr/${ARCH}-w64-mingw32/lib/libdsound.a \
    .

make -j $(nproc)
make install
