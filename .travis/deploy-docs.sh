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
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Fail out on error
set -eu -o pipefail

DOCS_FOLDER="./doc/html"

# Ensure docs exists
if [ ! -d "$DOCS_FOLDER" ]
then
    exit 1
fi

# Obtain git commit hash from HEAD
GIT_CHASH=$(git rev-parse HEAD)

# Push generated doxygen to GitHub pages
cd "$DOCS_FOLDER"

git --quiet init
git config user.name "Travis CI"
git config user.email "qTox@users.noreply.github.com"

git add .
git commit --quiet -m "Deploy to GH pages from commit: $GIT_CHASH"

echo "Pushing to GH pages..."
git push --force --quiet "https://${GH_TOKEN}@github.com/qTox/doxygen.git" master:gh-pages &> /dev/null
