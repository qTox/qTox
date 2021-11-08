#!/bin/bash

set -euo pipefail

AITOOL_HASH=effcebc1d81c5e174a48b870cb420f490fb5fb4d

git clone -b master --single-branch --recursive \
https://github.com/AppImage/AppImageKit .

git checkout "$AITOOL_HASH"
git submodule update --init --recursive
