#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later AND MIT
#     Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#     Copyright (c) 2021 by The qTox Project Contributors

set -euo pipefail

"$(dirname $(realpath "$0"))/download/download_mingw_ldd.sh"

cp -a mingw_ldd/mingw_ldd.py /usr/local/bin/mingw-ldd.py
