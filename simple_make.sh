#!/usr/bin/env bash

#   Copyright © 2019 by The qTox Project Contributors
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


set -eu -o pipefail

apt_install() {
    local apt_packages=(
        automake
        autotools-dev
        build-essential
        check
        checkinstall
        cmake
        git
        libavdevice-dev
        libexif-dev
        libgdk-pixbuf2.0-dev
        libgtk2.0-dev
        libopenal-dev
        libopus-dev
        libqrencode-dev
        libqt5opengl5-dev
        libqt5svg5-dev
        libsodium-dev
        libtool
        libvpx-dev
        libxss-dev
        qrencode
        qt5-default
        qttools5-dev
        qttools5-dev-tools
        libsqlcipher-dev
    )

    sudo apt-get install "${apt_packages[@]}"
}

pacman_install() {
    local pacman_packages=(
        base-devel
        git
        libsodium
        libvpx
        libxss
        openal
        opus
        qrencode
        qt5
    )
    sudo pacman -S --needed "${pacman_packages[@]}"
}

dnf_install() {
    local dnf_group_packages=(
        'Development Tools'
        'C Development Tools and Libraries'
    )
    sudo dnf group install "${dnf_group_packages[@]}"

    # pure Fedora doesn't have what it takes to compile qTox (ffmpeg)
    local fedora_version=$(rpm -E %fedora)
    local dnf_rpmfusion_package=(
        http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$fedora_version.noarch.rpm
    )
    sudo dnf install "$dnf_rpmfusion_package"

    local dnf_packages=(
        ffmpeg-devel
        gdk-pixbuf2-devel
        git
        glib2-devel
        gtk2-devel
        kf5-sonnet-devel
        libconfig-devel
        libexif-devel
        libsodium-devel
        libvpx-devel
        libXScrnSaver-devel
        openal-soft-devel
        openssl-devel
        opus-devel
        qrencode-devel
        qt5-devel
        qt5-qtdoc
        qt5-qtsvg
        qt5-qtsvg-devel
        qt5-qttools-devel
        qtsingleapplication-qt5
        readline-devel
        sqlcipher-devel
        sqlite-devel
    )
    sudo dnf install "${dnf_packages[@]}"
}

# Fedora by default doesn't include libs in /usr/local/lib so add it
fedora_locallib() {
    local llib_file="/etc/ld.so.conf.d/locallib.conf"
    local llib_lines=("/usr/local/lib/" "/usr/local/lib64/")

    # check whether needed line already exists
    is_locallib() {
        grep -q "^$1\$" "$llib_file"
    }

    # add each line only if it doesn't exist
    for llib_line in "${llib_lines[@]}"; do\
        is_locallib "$llib_line" \
       	    || echo "$llib_line" \
                | sudo tee -a "$llib_file";
    done
}

zypper_install() {
    local zypper_packages=(
        +pattern:devel_basis
        cmake
        git
        libavcodec-devel
        libavdevice-devel
        libopus-devel
        libexif-devel
        libQt5Concurrent-devel
        libqt5-linguist
        libqt5-linguist-devel
        libQt5Network-devel
        libQt5OpenGL-devel
        libqt5-qtbase-common-devel
        libqt5-qtsvg-devel
        libQt5Test-devel
        libQt5Xml-devel
        libsodium-devel
        libvpx-devel
        libXScrnSaver-devel
        openal-soft-devel
        qrencode-devel
        sqlcipher-devel
    )

    # if not sudo is installed, e.g. in docker image, install it
    command -v sudo || zypper in sudo

    sudo zypper in "${zypper_packages[@]}"
}

main() {
    local BOOTSTRAP_ARGS=""
    if command -v zypper && [ -f /etc/products.d/openSUSE.prod ]
    then
        zypper_install
    elif command -v apt-get
    then
        apt_install
    elif command -v pacman
    then
        pacman_install
    elif command -v dnf
    then
        dnf_install
        fedora_locallib
        export PKG_CONFIG_PATH="${PKG_CONFIG_PATH-}:/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig"
        BOOTSTRAP_ARGS="--without-sqlcipher"
    else
        echo "Unknown package manager, attempting to compile anyways"
    fi

    ./bootstrap.sh ${BOOTSTRAP_ARGS}
    mkdir -p _build
    cd _build
    cmake ../
    make -j$(nproc)
}
main
