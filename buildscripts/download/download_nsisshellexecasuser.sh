#!/bin/bash


set -euo pipefail

NSISSHELLEXECASUSER_HASH=8fc19829e144716a422b15a85e718e1816fe561de379b2b5ae87ef9017490799

source "$(dirname "$0")/common.sh"

download_file http://nsis.sourceforge.net/mediawiki/images/c/c7/ShellExecAsUser.zip

if ! check_sha256 "$NSISSHELLEXECASUSER_HASH" ShellExecAsUser.zip; then
    exit 1
fi

unzip ShellExecAsUser.zip
rm ShellExecAsUser.zip
