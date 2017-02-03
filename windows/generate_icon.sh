# Generates qtox.ico from qtox.svg
#
# Dependencies:
#   base64 8.25
#   ImageMagick 7.0.4
#   icoutils 0.31.0
#
base64 -d <<< R0lGODlhBAABAPEAAAAAAAAAAICAgMDAwCH5BAEAAAAALAAAAAAEAAEAAAIDjAYFADs= > pal.gif
convert -background none -depth 8 qtox.svg qtox_256_256_32.png
convert -resize 64x64 qtox_256_256_32.png qtox_64_64_32.png
convert -resize 48x48 qtox_256_256_32.png qtox_48_48_32.png
convert -resize 32x32 qtox_256_256_32.png qtox_32_32_32.png
convert -resize 24x24 qtox_256_256_32.png qtox_24_24_32.png
convert -resize 16x16 qtox_256_256_32.png qtox_16_16_32.png
convert +dither qtox_48_48_32.png png8:qtox_48_48_8.png
convert +dither qtox_32_32_32.png png8:qtox_32_32_8.png
convert +dither qtox_16_16_32.png png8:qtox_16_16_8.png
convert +dither -remap pal.gif qtox_32_32_8.png png8:qtox_32_32_1.png
convert +dither -remap pal.gif qtox_16_16_8.png png8:qtox_16_16_1.png
convert -modulate 99.5 -strip qtox_256_256_32.png qtox_256_256_32.png
convert -modulate 99.5 qtox_64_64_32.png qtox_64_64_32.png
convert -modulate 99.5 qtox_48_48_32.png qtox_48_48_32.png
convert -modulate 99.5 qtox_32_32_32.png qtox_32_32_32.png
convert -modulate 99.5 qtox_24_24_32.png qtox_24_24_32.png
convert -modulate 99.5 qtox_16_16_32.png qtox_16_16_32.png
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
icotool -l qtox.ico
