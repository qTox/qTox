#!/usr/bin/env bash

#   Copyright © 2019 by The qTox Project Contributors
#
#   This file is part of qTox, a Qt-based graphical interface for Tox.
#   qTox is libre software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   qTox is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with qTox.  If not, see <http://www.gnu.org/licenses/>

set -e -u pipefail

readonly SCRIPT_DIR=$(dirname $(readlink -f $0))

if [[ $(id -u) != 0 ]]
then
    >&2 echo "Please run as root."
    exit 1
fi

if [[ -z $(which apparmor_parser) ]]
then
   >&2 echo "AppArmor not found."
   exit 1
fi

echo "Copying AppArmor files..."
[[ ! -d "/etc/apparmor.d/tunables/usr.bin.qtox.d/" ]] && mkdir -v "/etc/apparmor.d/tunables/usr.bin.qtox.d/"
cp -v "${SCRIPT_DIR}/tunables/usr.bin.qtox" "/etc/apparmor.d/tunables/"
cp -v "${SCRIPT_DIR}/usr.bin.qtox" "/etc/apparmor.d/"
touch "/etc/apparmor.d/local/usr.bin.qtox"

echo "Restarting AppArmor..."
systemctl restart apparmor

echo "Done."

