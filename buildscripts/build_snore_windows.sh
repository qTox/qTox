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

"$(dirname "$0")"/download/download_snore.sh

cmake -DCMAKE_INSTALL_PREFIX=/windows/ \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_daemon=OFF \
      -DBUILD_settings=OFF \
      -DBUILD_snoresend=OFF \
      -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
      .

make -j $(nproc)
make install
