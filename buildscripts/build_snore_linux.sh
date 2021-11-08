#!/bin/bash

set -euo pipefail

usage ()
{
    echo "Build/install snore for linux"
    echo "Usage: $0 [--system-install]"
    echo "--system-install: Install to /usr instead of /usr/local"
}

SYSTEM_INSTALL=0
while (( $# > 0 )); do
    case $1 in
    --system-install) SYSTEM_INSTALL=1; shift ;;
    --help|-h) usage; exit 1 ;;
    *) echo "Unexpected argument $1"; usage; exit 1 ;;
    esac
done

"$(dirname "$0")"/download/download_snore.sh

# Snore needs to be installed into /usr for linuxdeployqt to find it
if [ $SYSTEM_INSTALL -eq 1 ]; then
    INSTALL_PREFIX_ARGS="-DCMAKE_INSTALL_PREFIX=/usr"
else
    INSTALL_PREFIX_ARGS=""
fi

cmake -DCMAKE_BUILD_TYPE=Release $INSTALL_PREFIX_ARGS

make -j $(nproc)
make install
