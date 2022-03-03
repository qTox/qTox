#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
#     Copyright (c) 2022 by The qTox Project Contributors


set -euo pipefail

usage()
{
    # note: this is the usage from the build script's context, so the usage
    # doesn't include --dep argument, since that comes from the build script
    # itself.
    echo "Download and build $DEP_NAME for the Windows cross compiling environment"
    echo "Usage: $0 --arch {win64|win32}"
}

parse_arch()
{
    while (( $# > 0 )); do
    case $1 in
        --arch) SCRIPT_ARCH=$2; shift 2 ;;
        --dep) DEP_NAME=$2; shift 2 ;;
        -h|--help) usage; exit 1 ;;
        *) echo "Unexpected argument $1"; usage; exit 1;;
    esac
    done

    if [ "${SCRIPT_ARCH}" == "win32" ] || [ "${SCRIPT_ARCH}" == "win64" ]; then
        if [ "${SCRIPT_ARCH}" == "win32" ]; then
            local ARCH="i686"
        elif [ "${SCRIPT_ARCH}" == "win64" ]; then
            local ARCH="x86_64"
        fi
        DEP_PREFIX='/windows/'
        MINGW_ARCH="${ARCH}"
        HOST_OPTION="--host=${MINGW_ARCH}-w64-mingw32"
        MAKE_JOBS="$(nproc)"
        CMAKE_TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake"
    else
        echo "Unexpected arch ${SCRIPT_ARCH}"
        usage
        exit 1
    fi
    export PKG_CONFIG_PATH="${DEP_PREFIX}/lib/pkgconfig"
}
