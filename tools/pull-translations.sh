#!/bin/bash
#
#    Copyright © 2016 Daniel Martí <mvdan@mvdan.cc>
#    Copyright © 2016 Zetok Zalbavar <zetok@openmailbox.org>
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

# This script pulls translations from weblate.
#
# It squashes all the changes into a single commit. This removes authorship
# from the changes, which is given to the Translatebot, so to keep the names
# they are grabbed from git log and added to the commit message.
#
# Note that this will apply changes and commit them! Make sure to not have
# uncommited changes when running this script.
#
# Note 2: after applying changes, files will be cleaned up before committing.

REMOTE="weblate"
REMOTE_URL="git://git.weblate.org/qtox.git"
REMOTE_BRANCH="master"

AUTHOR="qTox translations <>"

if ! git ls-remote --exit-code $REMOTE >/dev/null 2>/dev/null; then
    git add "$REMOTE" "$REMOTE_URL"
    git fetch "$REMOTE"
fi

ref="${REMOTE}/${REMOTE_BRANCH}"
diff="HEAD...$ref -- translations/*.ts"

authors=$(git log --format="%s %an" $diff | \
	sed 's/Translated using Weblate (\(.*\)) \(.*\)/\2||\1/' | sort -f -u | column -s '||' -t)

git diff $diff | git apply

# clean up before committing
bash tools/update-translation-files.sh

git add translations/*

git commit --author "$AUTHOR" -m "feat(l10n): Update translations from Weblate

Translators:

$authors"
