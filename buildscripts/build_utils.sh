#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
#     Copyright (c) 2022 by The qTox Project Contributors


set -euo pipefail

MACOS_MINIMUM_SUPPORTED_VERSION=10.15

usage()
{
    # note: this is the usage from the build script's context, so the usage
    # doesn't include --dep argument, since that comes from the build script
    # itself.
    echo "Download and build $DEP_NAME for the Windows cross compiling environment"
    echo "Usage: $0 --arch {$1}"
}

assert_supported()
{
    for supported in $2; do
        if [ "$1" == "$supported" ]; then
            return
        fi
    done
    usage "$2"
    exit 1
}

parse_arch()
{
    while (( $# > 0 )); do
    case $1 in
        --arch) SCRIPT_ARCH=$2; shift 2 ;;
        --dep) DEP_NAME=$2; shift 2 ;;
        --supported) SUPPORTED=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
    done

    assert_supported "${SCRIPT_ARCH}" "${SUPPORTED}"

    if [ "${SCRIPT_ARCH}" == "win32" ] || [ "${SCRIPT_ARCH}" == "win64" ]; then
        if [ "${SCRIPT_ARCH}" == "win32" ]; then
            MINGW_ARCH="i686"
        elif [ "${SCRIPT_ARCH}" == "win64" ]; then
            MINGW_ARCH="x86_64"
        fi
        DEP_PREFIX='/windows/'
        HOST_OPTION="--host=${MINGW_ARCH}-w64-mingw32"
        CROSS_LDFLAG=""
        CROSS_CFLAG=""
        CROSS_CPPFLAG=""
        MAKE_JOBS="$(nproc)"
        CMAKE_TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake"
    elif [ "${SCRIPT_ARCH}" == "macos" ]; then
        DEP_PREFIX="$(realpath $(dirname $(realpath ${BASH_SOURCE[0]}))/../local-deps)"
        mkdir -p $DEP_PREFIX
        HOST_OPTION=''
        CROSS_LDFLAG="-mmacosx-version-min=$MACOS_MINIMUM_SUPPORTED_VERSION"
        CROSS_CFLAG="-mmacosx-version-min=$MACOS_MINIMUM_SUPPORTED_VERSION"
        CROSS_CPPFLAG="-mmacosx-version-min=$MACOS_MINIMUM_SUPPORTED_VERSION"
        MAKE_JOBS="$(sysctl -n hw.ncpu)"
        CMAKE_TOOLCHAIN_FILE=""
    else
        echo "Unexpected arch ${SCRIPT_ARCH}"
        usage
        exit 1
    fi
    export PKG_CONFIG_PATH="${DEP_PREFIX}/lib/pkgconfig"
}
