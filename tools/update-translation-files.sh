#!/bin/bash
#
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

# Script for updating translation files.  Should be ran after applying commit
# to clean translation up or after modifying translation.
#
# NOTE: if latest commit is a translation from Weblate, it amends commit!

# Usage:
#   ./tools/$script_name [ALL|lang]

set -eu -o pipefail

readonly COMMIT_MSG="chore(i18n): update translation files for Weblate"
readonly LUPDATE_CMD="lupdate -pro qtox.pro -no-obsolete -locations none -ts"

update_all() {
    local git_cmd="git commit -S -m $COMMIT_MSG"
    for translation in translations/*.ts
    do
        $LUPDATE_CMD "$translation"
    done

    git add translations/*.ts
    $git_cmd
}

last_commit_title() {
    git log --format=format:%s HEAD~1..HEAD
}

is_last_commit_transl() {
    last_commit_title \
    | grep -q '^feat(l10n): update .* translation from Weblate$'
}

update_file() {
    local file="translations/$@.ts"
    local git_cmd="git commit -S"
    if is_last_commit_transl
    then
        git_cmd="$git_cmd --amend"
    fi

    $LUPDATE_CMD "$file"
    $git_cmd "$file"
}


main() {
    if [[ "$@" = "ALL" ]]
    then
        update_all
    else
        update_file $@
    fi
}
main $@
