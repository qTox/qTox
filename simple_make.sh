#!/usr/bin/env bash

if which apt-get; then
    sudo apt-get install \
        git build-essential qt5-qmake qt5-default qttools5-dev-tools \
        libqt5opengl5-dev libqt5svg5-dev libopenal-dev libopencv-dev \
        libxss-dev qrencode libqrencode-dev libtool autotools-dev \
        automake checkinstall check libopus-dev libvpx-dev libsodium-dev \
        libglib2.0-dev libgdk-pixbuf2.0-dev libgtk2.0-dev
elif which pacman; then
    sudo pacman -S --needed \
        git base-devel qt5 opencv openal libxss qrencode opus libvpx \
        libsodium
elif which dnf; then
    sudo dnf group install \
        "Development Tools"
    sudo dnf install \
        git qt-devel qt-doc qt-creator qt5-qtsvg opencv-devel \
        openal-soft-devel libXScrnSaver-devel qrencode-devel
else
    echo "Unknown package manager, attempting to compile anyways"
fi

./bootstrap.sh
qmake
make
