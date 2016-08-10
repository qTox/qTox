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


# Obtain doxygen
sudo apt-get install doxygen

CONFIG_FILE="doxygen.conf"
CONFIG_FILE_TMP="$CONFIG_FILE.tmp.autogen"

GIT_DESC=$(git describe --tags 2> /dev/null)
GIT_CHASH=$(git rev-parse HEAD)

# Create temporary config file
cp "$CONFIG_FILE" "$CONFIG_FILE_TMP"

# Append git version to doxygen version string
echo "PROJECT_NUMBER = \"Version: $GIT_DESC | Commit: $GIT_CHASH\"" >> "$CONFIG_FILE_TMP"

# Generate documentation
echo "Generating documentation..."
echo

doxygen "$CONFIG_FILE_TMP"
rm "$CONFIG_FILE_TMP"

# Push generated doxygen to GitHub pages
cd ./doc/html/

git init
git config user.name "Travis CI"
git config user.email "qTox@users.noreply.github.com"

git add .
git commit -m "Deploy to GH pages from commit: $GIT_CHASH"

echo "Pushing to GH pages..."
git push --force --quiet "https://${GH_TOKEN}@github.com/qTox/doxygen.git" master:gh-pages &> /dev/null
