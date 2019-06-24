#!/usr/bin/env bash

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

set -euo pipefail

. .travis/cirp/check_precondition.sh

if [ ! -z "$TRAVIS_TEST_RESULT" ] && [ "$TRAVIS_TEST_RESULT" != "0" ]
then
  echo "Build has failed, skipping publishing"
  exit 0
fi

if [ "$#" != "1" ]
then
  echo "Error: No arguments provided. Please specify a directory containing artifacts as the first argument."
  exit 1
fi

ARTIFACTS_DIR="$1"

. .travis/cirp/install.sh

ci-release-publisher store "$ARTIFACTS_DIR"
