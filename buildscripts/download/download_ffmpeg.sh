#!/bin/bash

set -euo pipefail

FFMPEG_VERSION=4.3.2
FFMPEG_HASH=46e4e64f1dd0233cbc0934b9f1c0da676008cad34725113fb7f802cfa84ccddb

source "$(dirname $0)"/common.sh

download_verify_extract_tarball \
    "https://www.ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.xz" \
    "${FFMPEG_HASH}"
