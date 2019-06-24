#!/bin/bash
#
#    Copyright © 2016-2019 by The qTox Project Contributors
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

# push latest tag to the `for-jenkins-release` branch to trigger a windows
# release build
# 
# should be run only when a new tag is pushed

set -eu -o pipefail

main() {
    # can already be cloned by the `build-gitstats` job
    if [[ ! -e qTox ]]
    then
        git clone https://github.com/qTox/qTox.git qTox
    fi

    cd qTox
    git checkout "$TRAVIS_BRANCH" -b for-jenkins-release
    git push --force "https://${GH_DEPLOY_JENKINS}@github.com/qTox/qTox.git"
}
main
