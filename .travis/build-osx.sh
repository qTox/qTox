#!/bin/bash
#
#    Copyright © 2016-2018 by The qTox Project Contributors
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
#

# Fail out on error
set -eu -o pipefail

# accelerate builds with ccache
install_ccache() {
    echo "Installing ccache ..."
    brew install ccache
}

# Build OSX
build() {
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -i
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -b
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -d
    bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -dmg
}

# check if binary was built
check() {
    local BIN_NAME="qTox.dmg"
    if [[ ! -s "$BIN_NAME" ]]
    then
        echo "There's no $BIN_NAME !"
        exit 1
    fi
}

main() {
    install_ccache
    build
    check
}
main
