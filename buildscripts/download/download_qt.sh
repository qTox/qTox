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

QT_MAJOR=5
QT_MINOR=12
QT_PATCH=12
QT_HASH=1979a3233f689cb8b3e2783917f8f98f6a2e1821a70815fb737f020cd4b6ab06

source "$(dirname "$(realpath "$0")")/common.sh"

download_verify_extract_tarball \
    https://download.qt.io/official_releases/qt/${QT_MAJOR}.${QT_MINOR}/${QT_MAJOR}.${QT_MINOR}.${QT_PATCH}/single/qt-everywhere-src-${QT_MAJOR}.${QT_MINOR}.${QT_PATCH}.tar.xz \
    "${QT_HASH}"
