#!/bin/bash

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
  local URL="$1"
  curl -L --connect-timeout 10 -O "$URL"
}

download_verify_extract_tarball()
{
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
