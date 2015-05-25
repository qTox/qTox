#!/bin/bash
# Builds qTox for Windows and prepares a redistribuable .zip archive
export PATH=/root/mxe/usr/bin:$PATH
tar xvf libs.tar.gz
tar xvf libtoxcore-win32.tar.gz -C libs
/root/mxe/usr/i686-pc-mingw32.shared/qt5/bin/qmake
make clean
make -j4 || { echo 'make failed, aborting' ; exit 1; }

rm -rf qTox-win32
rm -rf qTox-win32.zip
mkdir qTox-win32
tar xvf qtox-dlls.tar.gz -C qTox-win32
cp libs/bin/* qTox-win32
cp release/qtox.exe qTox-win32
rm -rf qTox-win32.zip
zip -r qTox-win32.zip qTox-win32
chmod 777 qTox-win32.zip
