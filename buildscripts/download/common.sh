#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

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
