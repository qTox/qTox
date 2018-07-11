#!/usr/bin/env bash

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
        libsodium-devel
        libtool
        libvpx-devel
        libXScrnSaver-devel
        openal-soft-devel
        openssl-devel
        opus-devel
        qrencode-devel
        qt5-qtsvg
        qt5-qtsvg-devel
        qt5-qttools-devel
        qt-creator
        qt-devel
        qt-doc
    )
    sudo dnf install "${dnf_packages[@]}"
}

# Fedora by default doesn't include libs in /usr/local/lib so add it
fedora_locallib() {
    local llib_file="/etc/ld.so.conf.d/locallib.conf"
    local llib_line="/usr/local/lib/"

    # check whether needed line already exists
    is_locallib() {
        grep -q "^$llib_line\$" "$llib_file"
    }

    # proceed only if line doesn't exist
    is_locallib \
        || echo "$llib_line" \
            | sudo tee -a "$llib_file"
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
    else
        echo "Unknown package manager, attempting to compile anyways"
    fi

    ./bootstrap.sh
    mkdir -p _build
    cd _build
    cmake ../
    make -j$(nproc)
}
main
