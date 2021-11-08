#!/bin/bash

set -euo pipefail

"$(dirname "$0")"/download/download_mingw_ldd.sh

cp -a mingw_ldd/mingw_ldd.py /usr/local/bin/mingw-ldd.py
