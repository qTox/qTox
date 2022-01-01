#!/bin/bash

#    Copyright © 2021 by The qTox Project Contributors
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

check_sha256()
{
  if uname | grep -q Darwin; then
    HASH_BIN="gsha256sum"
  else
    HASH_BIN="sha256sum"
  fi

  if ! ( echo "$1  $2" | $HASH_BIN -c --quiet --status - )
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

download_file()
{
  # Curl command wrapper to download a file to the CWD
  local URL="$1"
  curl -L --connect-timeout 10 -O "$URL"
}

download_verify_extract_tarball()
{
  # Downlaoads the tarball at URL, ensures it has an sha256sum of HASH and
  # extracts it to the CWD

  local URL="$1"
  local HASH="$2"

  # Try linux style mktemp and fallback on osx style
  TEMPDIR=$(mktemp -d -p . 2>/dev/null || mktemp -d -t qtox_download)

  if [ $? -ne 0 ]; then
    return 1
  fi

  TEMPDIR=$(cd "$TEMPDIR"; pwd -P)

  pushd "$TEMPDIR" >/dev/null || exit 1

  download_file "$URL"

  if ! check_sha256 "$HASH" *; then
    rm -fr "$TEMPDIR"
    return 1
  fi

  popd >/dev/null || exit 1

  tar -xf "$TEMPDIR"/* --strip-components=1

  rm -fr "$TEMPDIR"
}
