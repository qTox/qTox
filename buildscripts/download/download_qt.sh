#!/bin/bash

set -euo pipefail

QT_MAJOR=5
QT_MINOR=12
QT_PATCH=11
QT_HASH=0c4cdef158c61827d70d6111423166e2c62b539eaf303f36ad1d0aa8af900b95

source "$(dirname "$0")"/common.sh

download_verify_extract_tarball \
    https://download.qt.io/official_releases/qt/${QT_MAJOR}.${QT_MINOR}/${QT_MAJOR}.${QT_MINOR}.${QT_PATCH}/single/qt-everywhere-src-${QT_MAJOR}.${QT_MINOR}.${QT_PATCH}.tar.xz \
    "${QT_HASH}"
