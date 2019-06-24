#!/bin/bash

#    Copyright © 2016-2019 The qTox Project Contributors
#
#    This program is free software: you can redistribute it and/or modify
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

cd "$GITSTATS_DIR"
COMMIT=$(cd qTox && git describe)

git init --quiet
git config user.name "Travis CI"
git config user.email "qTox@users.noreply.github.com"

git add .
git commit --quiet -m "Deploy to GH pages from commit: $COMMIT"

echo "Pushing to GH pages..."
git push --force --quiet "https://${GH_TOKEN_GITSTATS}@github.com/qTox/gitstats.git" master:gh-pages &> /dev/null
