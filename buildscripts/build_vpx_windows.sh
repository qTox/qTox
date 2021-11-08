#!/bin/bash

set -euo pipefail

usage()
{
    echo "Download and build vpx for the windows cross compiling environment"
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

"$(dirname "$0")"/download/download_vpx.sh

if [ "${ARCH}" == "x86_64" ]; then
    ARCH_FLAGS="-fno-asynchronous-unwind-tables"
    VPX_ARCH="x86_64-win64-gcc"
elif [ "${ARCH}" == "i686" ]; then \
    ARCH_FLAGS="-fno-asynchronous-unwind-tables"
    VPX_ARCH="x86-win32-gcc"
else
    exit 1
fi

patch -Np1 < "$(dirname "$0")"/patches/vpx.patch

CFLAGS=${ARCH_FLAGS} CROSS="${ARCH}-w64-mingw32-" \
    ./configure --target="${VPX_ARCH}" \
        --prefix=/windows/ \
        --enable-shared \
        --disable-static \
        --enable-runtime-cpu-detect \
        --disable-examples \
        --disable-tools \
        --disable-docs \
        --disable-unit-tests

make -j $(nproc)
make install
