#!/bin/bash

#   Copyright © 2017-2019 The qTox Project Contributors
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

# Generates qtox.ico from qtox.svg
#
# Dependencies:
#   base64 8.25
#   ImageMagick 7.0.4
#   icoutils 0.31.0

# Black, gray and transparent colors from Windows 16-color palette
base64 -d <<< R0lGODlhBAABAPEAAAAAAAAAAICAgMDAwCH5BAEAAAAALAAAAAAEAAEAAAIDjAYFADs= > pal.gif
# Generate 32 bpp images
convert -background none -depth 8 qtox.svg qtox_256_256_32.png
convert -resize 64x64 qtox_256_256_32.png qtox_64_64_32.png
convert -resize 48x48 qtox_256_256_32.png qtox_48_48_32.png
convert -resize 32x32 qtox_256_256_32.png qtox_32_32_32.png
convert -resize 24x24 qtox_256_256_32.png qtox_24_24_32.png
convert -resize 16x16 qtox_256_256_32.png qtox_16_16_32.png
# Generate 8 bpp images
convert +dither qtox_48_48_32.png png8:qtox_48_48_8.png
convert +dither qtox_32_32_32.png png8:qtox_32_32_8.png
convert +dither qtox_16_16_32.png png8:qtox_16_16_8.png
# Generate 1 bpp images
convert +dither -remap pal.gif qtox_32_32_8.png png8:qtox_32_32_1.png
convert +dither -remap pal.gif qtox_16_16_8.png png8:qtox_16_16_1.png
# Hack for Windows XP file properties page
convert -modulate 99.5 -strip qtox_256_256_32.png qtox_256_256_32.png
convert -modulate 99.5 qtox_64_64_32.png qtox_64_64_32.png
convert -modulate 99.5 qtox_48_48_32.png qtox_48_48_32.png
convert -modulate 99.5 qtox_32_32_32.png qtox_32_32_32.png
convert -modulate 99.5 qtox_24_24_32.png qtox_24_24_32.png
convert -modulate 99.5 qtox_16_16_32.png qtox_16_16_32.png
# Build .ico file from .png images
icotool -c -t 32 \
  qtox_32_32_1.png \
  qtox_16_16_1.png \
  qtox_48_48_8.png \
  qtox_32_32_8.png \
  qtox_16_16_8.png \
  --raw=qtox_256_256_32.png \
  qtox_64_64_32.png \
  qtox_48_48_32.png \
  qtox_32_32_32.png \
  qtox_24_24_32.png \
  qtox_16_16_32.png \
  > qtox.ico
# Show debug information
icotool -l qtox.ico
