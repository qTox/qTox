#!/bin/bash

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

# Fail out on error
set -eu -o pipefail

# Extract html documentation directory from doxygen configuration
OUTPUT_DIR_CFG=( $(grep 'OUTPUT_DIRECTORY' "$DOXYGEN_CONFIG_FILE") )
HTML_OUTPUT_CFG=( $(grep 'HTML_OUTPUT' "$DOXYGEN_CONFIG_FILE") )

DOCS_DIR="./${OUTPUT_DIR_CFG[2]}/${HTML_OUTPUT_CFG[2]}/"

# Ensure docs exists
if [ ! -d "$DOCS_DIR" ]
then
    echo "Docs deploy failing, no $DOCS_DIR present."
    exit 1
fi

# Obtain git commit hash from HEAD
GIT_CHASH=$(git rev-parse HEAD)

# Push generated doxygen to GitHub pages
cd "$DOCS_DIR"

git init --quiet
git config user.name "qTox bot"
git config user.email "qTox@users.noreply.github.com"

git add .
git commit --quiet -m "Deploy to GH pages from commit: $GIT_CHASH"

echo "Pushing to GH pages..."
touch /tmp/access_key
chmod 600 /tmp/access_key
echo "$access_key" > /tmp/access_key
GIT_SSH_COMMAND="ssh -i /tmp/access_key" git push --force --quiet "git@github.com:qTox/doxygen.git" master:gh-pages
