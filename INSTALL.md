# Install Instructions
- [Dependencies](#dependencies)
- [Linux](#linux)
  - [Simple install](#simple-install)
    - [Generic](#generic)
    - [Arch](#arch-easy)
    - [Gentoo](#gentoo-easy)
    - [Slackware](#slackware-easy)
    - [FreeBSD](#freebsd-easy)
  - [Install git](#install-git)
    - [Arch](#arch-git)
    - [Debian](#debian-git)
    - [Fedora](#fedora-git)
    - [openSUSE](#opensuse-git)
    - [Ubuntu](#ubuntu-git)
  - [Clone qTox](#clone-qtox)
  - [GCC, Qt, FFmpeg, OpanAL Soft and qrencode](#other-deps)
    - [Arch](#arch-other-deps)
    - [Debian](#debian-other-deps)
    - [Fedora](#fedora-other-deps)
    - [openSUSE](#opensuse-other-deps)
    - [Slackware](#slackware-other-deps)
    - [Ubuntu >=15.04](#ubuntu-other-deps)
    - [Ubuntu >=16.04](#ubuntu-other-1604-deps)
  - [toxcore dependencies](#toxcore-dependencies)
    - [Arch](#arch-toxcore)
    - [Debian](#debian-toxcore)
    - [Fedora](#fedora-toxcore)
    - [openSUSE](#opensuse-toxcore)
    - [Slackware](#slackware-toxcore)
    - [Ubuntu >=15.04](#ubuntu-toxcore)
  - [filter_audio](#filter_audio)
  - [sqlcipher](#sqlcipher)
  - [Compile toxcore](#compile-toxcore)
  - [Compile qTox](#compile-qtox)
- [OS X](#osx)
- [Windows](#windows)

<a name="dependencies" />
## Dependencies

| Name          | Version     | Modules                                           |
|---------------|-------------|-------------------------------------------------- |
| Qt            | >= 5.3.0    | core, gui, network, opengl, sql, svg, widget, xml |
| GCC/MinGW     | >= 4.8      | C++11 enabled                                     |
| toxcore       | most recent | core, av                                          |
| FFmpeg        | >= 2.6.0    | avformat, avdevice, avcodec, avutil, swscale      |
| OpenAL Soft   | >= 1.16.0   |                                                   |
| filter_audio  | most recent |                                                   |
| qrencode      | >= 3.0.3    |                                                   |
| sqlcipher     | >= 3.2.0    |                                                   |
| libXScrnSaver | >= 1.2      |                                                   |
| pkg-config    | >= 0.28     |                                                   |


<a name="linux" />
## Linux
### Simple install
Easy qTox install is provided for variety of distributions:

* [Generic](#generic)
* [Arch](#arch)
* [Gentoo](#gentoo)
* [Slackware](#slackware)


#### Generic
There are available generic binaries for Linux:
* [**64 bit download**](https://build.tox.chat/view/Clients/job/qTox_build_linux_x86-64_release/lastSuccessfulBuild/artifact/qTox_build_linux_x86-64_release.tar.xz)
* [**32 bit download**](https://build.tox.chat/view/Clients/job/qTox_build_linux_x86_release/lastSuccessfulBuild/artifact/qTox_build_linux_x86_release.tar.xz)
* [**Armhf download**](https://build.tox.chat/job/qTox-qt5.4.2_build_linux_armhf_release/lastSuccessfulBuild/artifact/qTox-qt5.4.2_build_linux_armhf_release.tar.xz)


====

<a name="arch-easy" />
#### Arch

PKGBUILD is available in the `community` repo, to install:
```bash
pacman -S qtox
```

<a name="gentoo-easy" />
#### Gentoo

qTox is available in Gentoo.

To install:
```bash
emerge qtox
```


<a name="slackware-easy" />
#### Slackware

qTox SlackBuild and all of its dependencies can be found here:
http://slackbuilds.org/repository/14.1/network/qTox/

<a name="freebsd-easy" />
#### FreeBSD

A qTox port is available at ``net-im/qTox``. To build and install qTox:

```bash
cd /usr/ports/net-im/qTox/
make install
```

----

If your distribution is not listed, or you want / need to compile qTox, there
are provided instructions.


----

Most of the dependencies should be available through your package manger. You
may either follow the directions below, or simply run `./simple_make.sh` after
cloning this repository, which will attempt to automatically download
dependencies followed by compilation.

### Install git
In order to clone the qTox repository you need Git.


<a name="arch-git" />
#### Arch Linux
```bash
sudo pacman -S --needed git
```

<a name="debian-git" />
#### Debian
```bash
sudo apt-get install git
```

<a name="fedora-git" />
#### Fedora
*`yum` is now officially deprecated by `dnf`. using `yum` will redirect to `dnf`
on Fedora 21 and fail on future versions.*
```bash
sudo dnf install git
```

<a name="opensuse-git" />
#### openSUSE
```bash
sudo zypper install git
```

<a name="ubuntu-git" />
#### Ubuntu
```bash
sudo apt-get install git
```


### Clone qTox
Afterwards open a new terminal, change to a directory of your choice and clone
the repository:
```bash
cd /home/user/qTox
git clone https://github.com/tux3/qTox.git qTox
```

The following steps assumes that you cloned the repository at `/home/user/qTox`.
If you decided to choose another location, replace corresponding parts.


<a name="other-deps" />
### GCC, Qt, FFmpeg, OpanAL Soft and qrencode

<a name="arch-other-deps" />
#### Arch Linux
```bash
sudo pacman -S --needed base-devel qt5 openal libxss qrencode ffmpeg
```


<a name="debian-other-deps" />
#### Debian
**Note that only Debian >=8 stable (jessie) is supported.**

If you use stable, you have to add backports to your `sources.list` for FFmpeg
and others. Instructions here: http://backports.debian.org/Instructions/

```bash
sudo apt-get install build-essential qt5-qmake qt5-default qttools5-dev-tools \
libqt5opengl5-dev libqt5svg5-dev libopenal-dev libxss-dev qrencode \
libqrencode-dev libglib2.0-dev libgdk-pixbuf2.0-dev libgtk2.0-dev ffmpeg \
libsqlcipher-dev
```


<a name="fedora-other-deps" />
#### Fedora
**Note that sqlcipher is not included in Fedora(!).**

**This means that you have to compile sqlcipher yourself, otherwise compiling
qTox will fail.**
```bash
sudo dnf groupinstall "Development Tools"
# (can also use sudo dnf install @"Development Tools")
sudo dnf install qt-devel qt-doc qt-creator qt5-qtsvg qt5-qtsvg-devel \
openal-soft-devel libXScrnSaver-devel qrencode-devel ffmpeg-devel \
qtsingleapplication qt5-linguist gtk2-devel
```

**Go to [sqlcipher](#sqlcipher) section to compile it.**

<a name="opensuse-other-deps" />
#### openSUSE

```bash
sudo zypper install patterns-openSUSE-devel_basis libqt5-qtbase-common-devel \
libqt5-qtsvg-devel libqt5-linguist libQt5Network-devel libQt5OpenGL-devel \
libQt5Concurrent-devel libQt5Xml-devel libQt5Sql-devel openal-soft-devel \
qrencode-devel libXScrnSaver-devel libQt5Sql5-sqlite libffmpeg-devel \
sqlcipher-devel
```

<a name="slackware-other-deps" />
#### Slackware

List of all the qTox dependencies and their SlackBuilds can be found here:
http://slackbuilds.org/repository/14.1/network/qTox/


<a name="ubuntu-other-deps" />
#### Ubuntu >=15.04
```bash
sudo apt-get install build-essential qt5-qmake qt5-default qttools5-dev-tools \
libqt5opengl5-dev libqt5svg5-dev libopenal-dev libxss-dev qrencode \
libqrencode-dev libavutil-ffmpeg-dev libswresample-ffmpeg-dev \
libavcodec-ffmpeg-dev libswscale-ffmpeg-dev libavfilter-ffmpeg-dev \
libavdevice-ffmpeg-dev libglib2.0-dev libgdk-pixbuf2.0-dev libgtk2.0-dev \
libsqlcipher-dev
```

<a name="ubuntu-other-1604-deps" />
#### Ubuntu >=16.04:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default qttools5-dev-tools libqt5opengl5-dev libqt5svg5-dev libopenal-dev libxss-dev qrencode libqrencode-dev libavutil-dev libswresample-dev libavcodec-dev libswscale-dev libavfilter-dev libavdevice-dev libglib2.0-dev libgdk-pixbuf2.0-dev libgtk2.0-dev libsqlcipher-dev
```

### toxcore dependencies

Install all of the toxcore dependencies.

<a name="arch-toxcore" />
#### Arch Linux
```bash
sudo pacman -S --needed opus libvpx libsodium
```

<a name="debian-toxcore" />
#### Debian
```bash
sudo apt-get install libtool autotools-dev automake checkinstall check \
libopus-dev libvpx-dev libsodium-dev libavdevice-dev
```

<a name="fedora-toxcore" />
#### Fedora
```bash
sudo dnf install libtool autoconf automake check check-devel libsodium-devel \
opus-devel libvpx-devel
```

<a name="opensuse-toxcore" />
#### openSUSE
```bash
sudo zypper install libsodium-devel libvpx-devel libopus-devel \
patterns-openSUSE-devel_basis
```

<a name="slackware-toxcore" />
#### Slackware

List of all the toxcore dependencies and their SlackBuilds can be found
here: http://slackbuilds.org/repository/14.1/network/toxcore/


<a name="ubuntu-toxcore" />
#### Ubuntu >=15.04
```bash
sudo apt-get install libtool autotools-dev automake checkinstall check \
libopus-dev libvpx-dev libsodium-dev
```


### filter_audio
This step is  best done before compiling toxcore.

Now you can either follow the instructions at
https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#unix or use the
[`bootstrap.sh`](/bootstrap.sh) script. The script will automatically download
and install `toxcore` and `libfilteraudio`:
```bash
## in qTox directory
./bootstrap.sh # use -h or --help for more information

```
If you've used script, you can skip directly to [compiling qTox](#compile-qtox).

If you want to compile and install it manually:
```bash
git clone https://github.com/irungentoo/filter_audio
cd filter_audio
make -j$(nproc)
sudo make install
```

### sqlcipher

If you are not using Fedora, skip this section, and go directly to compiling
[**toxcore**](#toxcore-compiling).

This method automatically detects whether to link statically or dynamically,
depending on your system configs.
```bash
git clone https://github.com/sqlcipher/sqlcipher
cd sqlcipher
autoreconf -if
./configure
make -j$(nproc)
sudo make install
cd ..
```
If you wish to explicitly link sqlcipher
[statically](#statically-linked-sqlcipher) or
[dynamically](#dynamically-linked-sqlcipher).

#### Statically linked sqlcipher
```bash
git clone https://github.com/sqlcipher/sqlcipher
cd sqlpcipher
./configure --enable-tempstore=yes CFLAGS="-DSQLITE_HAS_CODEC" \
    LDFLAGS="/opt/local/lib/libcrypto.a"
make
sudo make install
cd ..
```

#### Dynamically linked sqlcipher
```bash
git clone https://github.com/sqlcipher/sqlcipher
cd sqlcipher
./configure --enable-tempstore=yes CFLAGS="-DSQLITE_HAS_CODEC" \
    LDFLAGS="-lcrypto"
make
sudo make install
cd ..
```

### Compile toxcore

Provided that you have all required dependencies installed, you can simply run:
```bash
git clone https://github.com/irungentoo/toxcore.git
cd toxcore
autoreconf -if
./configure
make -j$(nproc)
sudo make install
echo '/usr/local/lib/' | sudo tee -a /etc/ld.so.conf.d/locallib.conf
sudo ldconfig
```


### Compile qTox
**Make sure that all the dependencies are installed.**  If you experience
problems with compiling, it's most likely due to missing dependencies, so please
make sure that you did install *all of them*.

Run in qTox directory to compile:
```bash
qmake
make
```

Now you can start compiled qTox with `./qtox`

Congratulations, you've compiled qTox `:)`


#### openSUSE / Fedora

Note to Fedora users: check qt5 version before building default is 4.8 on fedora
21 / 22, everything up until `qmake-qt5` will build fine but then `qmake-qt5`
will freak out.
```bash
qmake-qt5
make
```

#### Debian / Ubuntu / Mint
If the compiling process stops with a missing dependency like:
`... libswscale/swscale.h missing` try:
```bash
apt-file search libswscale/swscale.h
```
And install the package that provides the missing file.
Start make again. Repeat if necessary until all dependencies are installed. If
you can, please note down all additional dependencies you had to install that
aren't listed here, and let us know what is missing `;)`


====

### Building packages

Alternately, qTox now has the experimental and probably-dodgy ability to package
itself (in `.deb` form natively, and `.rpm` form with
[alien](http://joeyh.name/code/alien/)).

After installing the required dependencies, run `bootstrap.sh` and then run the
`buildPackages.sh` script, found in the tools folder. It will automatically get
the packages necessary for building `.deb`s, so be prepared to type your
password for sudo.


<a name="osx" />
## OS X

Compiling qTox on OS X for development requires 3 tools:
[Xcode](https://developer.apple.com/xcode/),
[Qt 5.4+](http://www.qt.io/qt5-4/) and [homebrew](http://brew.sh).

### Automated Script
You can now set up your OS X system to compile qTox automatically thanks to the
script in: `./osx/qTox-Mac-Deployer-ULTIMATE.sh`

This script can be run independently of the qTox repo and is all that's needed
to build from scratch on OS X.

To use this script you must launch terminal which can be found:
`Applications > Utilities > Terminal.app`

If you wish to lean more you can run `./qTox-Mac-Deployer-ULTIMATE.sh -h`

#### First Run / Install
If you are running the script for the first time you will want to make sure your
system is ready. To do this simply run `./qTox-Mac-Deployer-ULTIMATE.sh -i` to
run you through the automated install set up.

After running the installation setup you are now ready to build qTox from
source, to do this simply run: `./qTox-Mac-Deployer-ULTIMATE.sh -b`

If there aren't any errors then you'll find a locally working qTox application
in your home folder under `~/qTox-Mac_Build`

#### Updating
If you want to update your application for testing purposes or you want to run a
nightly build setup then run: `./qTox-Mac-Deployer-ULTIMATE.sh -u` and follow
the prompts. (NOTE: If you know you updated the repos before running this hit Y)
followed by `./qTox-Mac-Deployer-ULTIMATE.sh -b` to build the application once
more. (NOTE: This will delete your previous build.)

#### Deploying
OS X requires an extra step to make the `qTox.app` file shareable on a system
that doesn't have the required libraries installed already.

If you want to share the build you've made with your other friends who use OS X
then simply run: `./qTox-Mac-Deployer-ULTIMATE.sh -d`

### Manual Compiling
#### Required Libraries
Install homebrew if you don't have it:
```bash
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

First, let's install the dependencies available via `brew`.
```bash
brew install git ffmpeg qrencode libtool automake autoconf check qt5 libvpx \
opus sqlcipher libsodium
```

Next, install [filter_audio](https://github.com/irungentoo/filter_audio) (you
may delete the directory it creates afterwards):
```bash
git clone https://github.com/irungentoo/filter_audio.git
cd filter_audio
sudo make install
cd ../
```

Next, install
[toxcore](https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#osx)

Then, clone qTox:
```bash
git clone https://github.com/tux3/qTox``
```

Finally, copy all required files. Whenever you update your brew packages, you
may skip all of the above steps and simply run the following commands:
```bash
cd ./git/qTox
sudo bash bootstrap-osx.sh
```

#### Compiling
You can build qTox with Qt Creator
[seperate download](http://www.qt.io/download-open-source/#section-6) or you can
hunt down the version of home brew qt5 your using in the
`/usr/local/Cellar/qt5/` directory. e.g.
`/usr/local/Cellar/qt5/5.5.1_2/bin/qmake` with `5.5.1_2` being the version of
Qt5 that's been installed.

With that; in your terminal you can compile qTox in the git dir:
```bash
/usr/local/Cellar/qt5/5.5.1_2/bin/qmake ./qtox.pro
```

Or a cleaner method would be to:
```bash
cd ./git/dir/qTox
mkdir ./build
cd build
/usr/local/Cellar/qt5/5.5.1_2/bin/qmake ../qtox.pro
```

#### Deploying
If you compiled qTox properly you can now deploy the `qTox.app` that's created
where you built qTox so you can distribute the package.

Using your qt5 homebrew installation from the build directory:
```bash
/usr/local/Cellar/qt5/5.5.1_2/bin/macdeployqt ./qTox.app
```

#### Running qTox
You've got 2 choices, either click on the qTox app that suddenly exists, or do
the following:
```bash
qtox.app/Contents/MacOS/qtox
```
Enjoy the snazzy CLI output as your friends and family congratulate you on
becoming a hacker

<a name="windows" />
## Windows

### Qt

Download the Qt online installer for Windows from
[qt.io](https://www.qt.io/download-open-source/). While installation you have
to assemble your Qt toolchain. Take the most recent version of Qt compiled with
MinGW. Although the installer provides its own bundled MinGW compiler toolchain
its recommend installing it separately because Qt is missing MSYS which is
needed to compile and install OpenAL. Thus you can - if needed - deselect the
tab `Tools`. The following steps assume that Qt is installed at `C:\Qt`. If you
decided to choose another location, replace corresponding parts.

### MinGW

Download the MinGW installer for Windows from
[sourceforge.net](http://sourceforge.net/projects/mingw/files/Installer/). Make
sure to install MSYS (a set of Unix tools for Windows). The following steps
assume that MinGW is installed at `C:\MinGW`. If you decided to choose another
location, replace corresponding parts. Check that the version of MinGW,
corresponds to the version of the QT component!

### Wget

Download the Wget installer for Windows from
http://gnuwin32.sourceforge.net/packages/wget.htm. Install them. The following
steps assume that Wget is installed at `C:\Program Files\GnuWin32\`. If you
decided to choose another location, replace corresponding parts.

### UnZip

Download the UnZip installer for Windows from
http://gnuwin32.sourceforge.net/packages/unzip.htm. Install it. The following
steps assume that UnZip is installed at `C:\Program Files\GnuWin32\`. If you
decided to choose another location, replace corresponding parts.

### Setting up Path

Add MinGW/MSYS/CMake binaries to the system path to make them globally
accessible. Open `Control Panel` -> `System and Security` -> `System` ->
`Advanced system settings` -> `Environment Variables...` (or run `sysdm.cpl`
select tab `Advanced system settings` -> button `Environment Variables`). In the
second box search for the `PATH` variable and press `Edit...`. The input box
`Variable value:` should already contain some directories. Each directory is
separated with a semicolon. Extend the input box by adding
`;C:\MinGW\bin;C:\MinGW\msys\1.0\bin;C:\Program Files (x86)\CMake 2.8\bin;C:\Program Files\GnuWin32\bin;C:\Program Files (x86)\GnuWin32\bin`.
The very first semicolon must only be added if it is missing. CMake may be added
by installer automatically.

### Cloning the Repository

Clone the repository (https://github.com/tux3/qTox.git) with your preferred  Git
client. [SmartGit](http://www.syntevo.com/smartgit/) is very nice for this task
(you may need to add the path to the `git.exe` system variable Path). The
following steps assume that you cloned the repository at `C:\qTox`. If you
decided to choose another location, replace corresponding parts.

### Getting dependencies
Run `bootstrap.bat` in cloned `C:\qTox` directory. Script will download rest of
dependencies compile them and put to appropriate directories.
