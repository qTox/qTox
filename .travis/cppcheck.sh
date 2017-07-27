#!/bin/bash
#
#    Copyright © 2017 by The qTox Project Contributors
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

cppcheck . -i test/ --enable=all -q --inline-suppr --error-exitcode=1 \
    -DENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND=1 \
    -DENABLE_SYSTRAY_UNITY_BACKEND=1 \
    -DENABLE_SYSTRAY_GTK_BACKEND=1 \
    -DQ_OS_WIN=1 -DQ_OS_LINUX=1 -DQ_OS_OSX=1 \
    -DQTOX_PLATFORM_EXT=1 \
    -DENABLE_CAPSLOCK_INDICATOR=1
