#!/bin/sh
CURRENT_DIR=$DIRSTACK
SOURCE_DIR="filter_audio/"
SOURCE_PATH="./../"
LIB_DIR="/usr/local/lib/"
INCLUDE_DIR="/usr/local/include/"

echo "Clone filter_audio from GitHub.com"
cd $SOURCE_PATH
git clone --quiet https://github.com/irungentoo/filter_audio.git $SOURCE_DIR

echo "Compile filter_audio"
cd $SOURCE_DIR
gcc -c -fPIC filter_audio.c aec/*.c agc/*.c ns/*.c other/*.c  -lm -lpthread

echo "Create shared object file"
gcc *.o -shared -o libfilteraudio.so

echo "Clean up"
rm *.o

echo "Install libfilteraudio.so"
sudo cp libfilteraudio.so $LIB_DIR

echo "Install include files"
sudo cp *.h $INCLUDE_DIR

echo "Finished."
cd $CURRENT_DIR
exit 1

