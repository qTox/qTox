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

# Install verifying the hash

# Get Python >=3.5
if [ "$TRAVIS_OS_NAME" == "osx" ]
then
  brew update

  # make sha256sum available
  export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"

  brew upgrade python || true

  python --version || true
  python3 --version || true
  pyenv versions || true

  cd .
  cd "$(mktemp -d)"
  virtualenv env -p python3
  set +u
  source env/bin/activate
  set -u
  cd -
else
  python --version || true
  python3 --version || true
  pyenv versions || true

  pyenv global 3.6
fi

pip install --upgrade pip

check_sha256()
{
  if ! ( echo "$1  $2" | sha256sum -c --status - )
  then
    echo "Error: sha256 of $2 doesn't match the known one."
    echo "Expected: $1  $2"
    echo -n "Got: "
    sha256sum "$2"
    exit 1
  else
    echo "sha256 matches the expected one: $1"
  fi
}

# Don't install again if already installed.
# OSX keeps re-installing it tough, as it uses a temp per-script virtualenv.
if ! pip list --format=columns | grep '^ci-release-publisher '
then
  cd .
  cd "$(mktemp -d)"
  VERSION="0.1.0"
  FILENAME="ci_release_publisher-$VERSION-py3-none-any.whl"
  HASH="384bd9e2b0dd344381c01948e567bb26151636d6fd9b70fc58ef94aeb6be9466"
  pip download ci_release_publisher==$VERSION
  check_sha256 "$HASH" "$FILENAME"
  pip install --no-index --find-links "$PWD" "$FILENAME"
  cd -
fi
