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

# Script to update the translations from Weblate. Works only if there are no
# merge conflicts. Assumes that working dir is a qTox git repo.
#
# Requires a GPG key to sign merge commits.

# usage:
#   ./$script 


set -e -o pipefail

readonly COMMIT_NAME="chore(l10n): update translations from Weblate

"

source_functions() {
    local fns_file="tools/lib/PR_bash.source"
    source $fns_file
}

# If there's no qTox Weblate remote, add it
add_remote_weblate() {
    local remote_url="https://hosted.weblate.org/git/tox/qtox/"
    local remote_name="weblate"

    is_remote_present $remote_name \
        || git remote add $remote_name "${remote_url}"
}

do_merge() {
    # update commits
    git fetch upstream
    git fetch weblate
    
    git checkout upstream/master
    
    local COMMIT_MESSAGE=`git shortlog upstream/master..weblate/master`
    
    # squash and merge    
    git merge --squash --no-edit weblate/master
    
    # update format for adapt to Qt translation format
    ./tools/deweblate-translation-file.sh ./translations/*.ts || true
    
    # commit
    git commit --no-edit -S -m "$COMMIT_NAME" -m "$COMMIT_MESSAGE"
}

main() {
    source_functions
    add_remote
    add_remote_weblate

    do_merge
    
    echo "You can now checkout the changes with:"
    echo ""
    echo "    git checkout -b <branch-name>"
    echo ""
}
main
