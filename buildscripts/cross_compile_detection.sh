#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
#     Copyright (c) 2022 by The qTox Project Contributors


set -euo pipefail

MACOS_MINIMUM_SUPPORTED_VERSION=10.14

parse_arch()
{
    while (( $# > 0 )); do
    case $1 in
        --arch) ARCH=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
    done

    if [ "$ARCH" == "macos" ]; then
        DEP_PREFIX="$(realpath $(dirname $(realpath ${BASH_SOURCE[0]}))/../local-deps)"
        mkdir -p $DEP_PREFIX
        HOST_OPTION=''
        CROSS_LDFLAG="-mmacosx-version-min=$MACOS_MINIMUM_SUPPORTED_VERSION"
        CROSS_CFLAG="-mmacosx-version-min=$MACOS_MINIMUM_SUPPORTED_VERSION"
        CROSS_CPPFLAG="-mmacosx-version-min=$MACOS_MINIMUM_SUPPORTED_VERSION"
        MAKE_JOBS="$(sysctl -n hw.ncpu)"
        CMAKE_TOOLCHAIN_FILE=""
    elif [ "$ARCH" == "i686" ] || [ "$ARCH" == "x86_64" ]; then
        DEP_PREFIX='/windows/'
        HOST_OPTION="--host=${ARCH}-w64-mingw32"
        CROSS_LDFLAG=''
        CROSS_CFLAG=''
        CROSS_CPPFLAG=''
        MAKE_JOBS="$(nproc)"
        CMAKE_TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake"
    else
        echo "Unexpected arch $ARCH"
        usage
        exit 1
    fi
    export PKG_CONFIG_PATH=$DEP_PREFIX/lib/pkgconfig
}
