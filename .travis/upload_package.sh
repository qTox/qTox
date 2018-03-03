#!/bin/bash
#
#    Copyright © 2018 by The qTox Project Contributors
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

if [ "x$1" == "xnightly" ]; then
    REP=qtox-nightly
    PKG=qtox_nightly-*-Linux.deb
else
    REP=qtox
    PKG=qtox-*-Linux.deb
fi

URL_BASE="https://api.bintray.com/content/qtox/qTox/$REP/$TRAVIS_BUILD_NUMBER"
PARAMS="deb_distribution=trusty;deb_component=main;deb_architecture=amd64;publish=1"
curl -T $PKG -uqtox:$BINTRAY_API_KEY "$URL_BASE/$PKG;$PARAMS"
