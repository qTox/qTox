#!/usr/bin/env bash

if which apt-get; then
    sudo apt-get install \
        git build-essential qt5-qmake qt5-default qttools5-dev-tools \
        libqt5opengl5-dev libqt5svg5-dev libopenal-dev libavdevice-dev \
        libxss-dev qrencode libqrencode-dev libtool autotools-dev \
        automake checkinstall check libopus-dev libvpx-dev libsodium-dev \
        libglib2.0-dev libgdk-pixbuf2.0-dev libgtk2.0-dev \
        libsqlcipher-dev
<<<<<<< a735c54a0ad24a2dd88a2fd0862bde49caa98f2a
elif which pacman; then
    sudo pacman -S --needed \
        git base-devel qt5 openal libxss qrencode opus libvpx libsodium
elif which dnf; then
    sudo dnf group install \
        "Development Tools"
    # pure Fedora doesn't have what it takes to compile qTox (ffmpeg)
    sudo dnf install \
        http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
    sudo dnf install \
        git qt-devel qt-doc qt-creator qt5-qtsvg qt5-qtsvg-devel \
        openal-soft-devel qt5-qttools-devel libXScrnSaver-devel \
        qrencode-devel opus-devel libvpx-devel glib2-devel gdk-pixbuf2-devel \
        gtk2-devel libsodium-devel ffmpeg-devel sqlite sqlite-devel
elif which zypper; then
    sudo zypper in \
        git patterns-openSUSE-devel_basis libqt5-qtbase-common-devel \
        libqt5-qtsvg-devel libqt5-linguist libQt5Network-devel \
        libQt5OpenGL-devel libQt5Concurrent-devel libQt5Xml-devel \
        libQt5Sql-devel openal-soft-devel qrencode-devel \
        libXScrnSaver-devel libQt5Sql5-sqlite libffmpeg-devel \
        libsodium-devel libvpx-devel libopus-devel \
        patterns-openSUSE-devel_basis sqlcipher-devel
else
    echo "Unknown package manager, attempting to compile anyways"
fi

./bootstrap.sh
if [ -e /etc/redhat-release -o -e /etc/zypp ]; then
    qmake-qt5
else
    qmake
fi
make -j$(nproc)
=======
        libtool
        libvpx-dev
        libxss-dev
        qrencode
        qt5-default
        qt5-qmake
        qttools5-dev-tools
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
        '@"development-tools"'
       
    )
    sudo dnf install "${dnf_group_packages[@]}"

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
        sqlite
        sqlite-devel
	sqlite-tcl
	intltool
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
        git
        libffmpeg-devel
        libopus-devel
        libQt5Concurrent-devel
        libqt5-linguist
        libQt5Network-devel
        libQt5OpenGL-devel
        libqt5-qtbase-common-devel
        libqt5-qtsvg-devel
        libQt5Sql5-sqlite
        libQt5Sql-devel
        libQt5Xml-devel
        libsodium-devel
        libvpx-devel
        libXScrnSaver-devel
        openal-soft-devel
        patterns-openSUSE-devel_basis
        patterns-openSUSE-devel_basis
        qrencode-devel
        sqlcipher-devel
    )
    sudo zypper in "${zypper_packages[@]}"
}

main() {
    if which apt-get
    then
        apt_install
    elif which pacman
    then
        pacman_install
    elif which dnf
    then
        dnf_install
        fedora_locallib
    elif which zypper
    then
        zypper_install
    else
        echo "Unknown package manager, attempting to compile anyways"
    fi

    ./bootstrap.sh
    if [ -e /etc/redhat-release -o -e /etc/zypp ]; then
        qmake-qt5
    else
        qmake
    fi
    make -j$(nproc)
}
main
>>>>>>>  On branch fedora_packages
