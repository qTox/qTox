#! /bin/bash

if which apt-get; then
    sudo apt-get install build-essential qt5-qmake qt5-default libopenal-dev libopencv-dev \
                         libtool autotools-dev automake checkinstall check libopus-dev libvpx-dev qttools5-dev-tools qtchooser libxss-dev libqt5svg5*
elif which pacman; then
    sudo pacman -S --needed base-devel qt5 opencv openal opus libvpx libxss qt5-svg
elif which yum; then
    sudo yum groupinstall "Development Tools"
    sudo yum install qt-devel qt-doc qt-creator opencv-devel openal-soft-devel libtool autoconf automake check check-devel libXScrnSaver-devel qt5-qtsvg
else
    echo "Unknown package manager, attempting to compile anyways"
fi

./bootstrap.sh
qmake SIMPLEMAKE=YES
make
