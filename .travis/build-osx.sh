#!/bin/bash
#
#    Copyright © 2016 by The qTox Project
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

# Fail out on error
set -e -o pipefail

# Build OSX
bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -i
bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -b
bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -d
# The following line can randomly fail due to travis emitting the error:
# "hdiutil: create failed - Resource busy"
bash ./osx/qTox-Mac-Deployer-ULTIMATE.sh -dmg
