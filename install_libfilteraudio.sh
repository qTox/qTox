#!/bin/sh

if [ -z $1 ]; then
  SOURCE_DIR="filter_audio/"
else
  SOURCE_DIR="$1/"
fi

if [ -z "$2" ]; then
  LIB_DIR="/usr/local/lib/"
  INCLUDE_DIR="/usr/local/include/"
else
  LIB_DIR="$2/lib/"
  INCLUDE_DIR="$2/include/"
fi

WINDOWS_VERSION=$(cmd.exe /c ver 2>/dev/null | grep "Microsoft Windows")
if [ ! -z "$WINDOWS_VERSION" ]; then
  EXT="dll"
  BIN_DIR="$2/bin/"
  STATIC_EXT="$EXT.a"
else
  BIN_DIR=$LIB_DIR
  EXT="so"
  STATIC_EXT="a"
fi

echo "Cloning filter_audio from GitHub.com"
git clone https://github.com/irungentoo/filter_audio.git $SOURCE_DIR
[ $? -eq 0 ] || exit 1

echo "Compiling filter_audio"
cd $SOURCE_DIR
gcc -c -fPIC filter_audio.c aec/*.c agc/*.c ns/*.c other/*.c  -lm -lpthread
[ $? -eq 0 ] || exit 1

echo "Creating shared object file"
if [ ! -z "$WINDOWS_VERSION" ]; then
  # This is for MingGW:
  gcc *.o -shared -o libfilteraudio.$EXT -Wl,--out-implib,libfilteraudio.$STATIC_EXT
  [ $? -eq 0 ] || exit 1
else
  # This is for rest of the world:
  make
  [ $? -eq 0 ] || exit 1
  ln libfilteraudio.$EXT.* libfilteraudio.so
fi

echo "Cleaning up"
rm *.o

muhcmd="cp libfilteraudio.$EXT $BIN_DIR"
[ -z "$2" ] && muhcmd="sudo $muhcmd"
echo "Installing libfilteraudio.so with $muhcmd"
$muhcmd || exit 1

muhcmd="cp libfilteraudio.$STATIC_EXT $LIB_DIR"
[ -z "$2" ] && muhcmd="sudo $muhcmd"
echo "Installing libfilteraudio.$STATIC_EXT with $muhcmd"
$muhcmd || exit 1

muhcmd="cp *.h $INCLUDE_DIR"
[ -z "$2" ] && muhcmd="sudo $muhcmd"
echo "Installing include files with $muhcmd"
$muhcmd || exit 1

echo "Finished."
