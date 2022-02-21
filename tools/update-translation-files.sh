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

# Script for updating translation files.  Should be ran after
# translatable strings are modified.
#
# Needed, since Weblate cannot do it automatically.

# Usage:
#   ./tools/$script_name [ALL|translation file]

set -eu -o pipefail

readonly LUPDATE_CMD="lupdate src -no-obsolete -locations none"

if [[ "$@" = "ALL" ]]
then
    for translation in translations/*.ts
    do
        $LUPDATE_CMD -ts "$translation"
    done
    $LUPDATE_CMD -pluralonly -ts translations/en.ts
else
    $LUPDATE_CMD "$@"
fi
