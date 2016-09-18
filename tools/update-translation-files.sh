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

# Script for updating translation files.  Should be ran after
# translatable strings are modified.
#
# Needed, since Weblate cannot do it automatically.

# Usage:
#   ./tools/$script_name [ALL|lang]

set -eu -o pipefail

readonly COMMIT_MSG="chore(i18n): update translation files for weblate"
readonly LUPDATE_CMD="lupdate -pro qtox.pro -no-obsolete -locations none -ts"

if [[ "$@" = "ALL" ]]
then
    for translation in translations/*.ts
    do
        $LUPDATE_CMD "$translation"
    done

    git add translations/*.ts
    git commit --author -m "$COMMIT_MSG"
else
    $LUPDATE_CMD "translations/$@.ts"
fi
