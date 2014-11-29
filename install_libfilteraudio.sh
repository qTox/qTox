#!/bin/sh
echo "Clone filter_audio from GitHub.com"
git clone --quiet https://github.com/irungentoo/filter_audio.git
cd filter_audio
echo "Compile filter_audio"
gcc -c -fPIC filter_audio.c aec/*.c agc/*.c ns/*.c other/*.c  -lm -lpthread
echo "Create shared object file"
gcc *.o -shared -o libfilteraudio.so
echo "Clean up"
rm *.o
echo "Install libfilteraudio.so"
sudo cp ./libfilteraudio.so /usr/local/lib/
cd ..
echo "Finished."

