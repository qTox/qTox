#!/bin/bash

set -euo pipefail

while (( $# > 0 )); do
    case $1 in
    --minimal) MINIMAL=1 ; shift ;;
    --full) MINIMAL=0; shift ;;
    --build-type) BUILD_TYPE=$2; shift 2 ;;
    *) echo "Unexpected argument $1"; exit 1 ;;
    esac

done

SRCDIR=/qtox
BUILDDIR=/qtox/build

rm -fr "$BUILDDIR"
mkdir -p "$BUILDDIR"
cd "$BUILDDIR"

if [ $MINIMAL -eq 1 ]; then
    cmake "$SRCDIR" \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DSMILEYS=DISABLED \
        -DSTRICT_OPTIONS=ON \
        -DSPELL_CHECK=OFF
else
    cmake "$SRCDIR" \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DUPDATE_CHECK=ON \
        -DSTRICT_OPTIONS=ON \
        -DCODE_COVERAGE=ON \
        -DDESKTOP_NOTIFICATIONS=ON
fi

cmake --build . -- -j $(nproc)

cmake --build . --target test

echo "Checking whether files processed by CMake have been committed..."
echo ""
# ↓ `0` exit status only if there are no changes
git diff --exit-code
