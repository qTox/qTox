#!/bin/bash

#   Copyright © 2016 Zetok Zalbavar <zetok@openmailbox.org>
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

# Script to run to clean up translations from Weblate.
#
# It detects whether current HEAD commit seems to be a translation from
# Weblate, and if it is, it proceeds to update the translation commit by
# amending it.
#
# Note: the script assumes:
#  * there is only 1 translation file modified
#  * translation commit strictly adheres to consistent commit naming used
#  * script is called from the root of repo
#
# If those assumptions aren't met, the end result won't be what you want.

set -eu -o pipefail

# get title of the last commit
get_commit_title() {
    git log --format=format:%s HEAD~1..HEAD
}

# get the whole commit message
get_whole_commit_name() {
    git log --format=format:%B HEAD~1..HEAD
}

# bool, whether HEAD commit is a weblate translation
is_webl_tr() {
    local re='^feat\(l10n\): update .* translation from Weblate$'
    local commit=$(get_commit_title)
    [[ $commit =~ $re ]]
}

# get filename of file to be updated
get_filename() {
    local raw=( $(git log --raw | egrep '^:[[:digit:]]{6}' | head -n1) )
    local re='^translations/.+\.ts$'
    [[ ${raw[5]} =~ $re ]] # check if that's actually right, if not, fail here
    echo ${raw[5]}
}

# call the other script to update && amend
update() {
    local file=$(get_filename)
    local commit_msg=$(get_whole_commit_name)
    ./tools/update-translation-files.sh "$file"
    git commit -S --amend -m "$commit_msg" "$file"
}


main() {
    is_webl_tr \
    && update
}
main
