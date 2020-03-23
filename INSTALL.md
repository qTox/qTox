# Install Instructions
- [Dependencies](#dependencies)
- [Linux](#linux)
  - [Simple install](#simple-install)
    - [Arch](#arch-easy)
    - [Fedora](#fedora-easy)
    - [Gentoo](#gentoo-easy)
    - [openSUSE](#opensuse-easy)
    - [Slackware](#slackware-easy)
  - [Install git](#install-git)
    - [Arch](#arch-git)
    - [Debian](#debian-git)
    - [Fedora](#fedora-git)
    - [openSUSE](#opensuse-git)
    - [Ubuntu](#ubuntu-git)
  - [Clone qTox](#clone-qtox)
  - [GCC, Qt, FFmpeg, OpenAL Soft and qrencode](#other-deps)
    - [Arch](#arch-other-deps)
    - [Debian](#debian-other-deps)
    - [Fedora](#fedora-other-deps)
    - [openSUSE](#opensuse-other-deps)
    - [Slackware](#slackware-other-deps)
    - [Ubuntu](#ubuntu-other-deps)
  - [Compile toxcore](#compile-toxcore)
  - [Compile qTox](#compile-qtox)
  - [Security hardening with AppArmor](#security-hardening-with-apparmor)
- [BSD](#bsd)
  - [FreeBSD](#freebsd-easy)
- [OS X](#osx)
- [Windows](#windows)
  - [Cross-compile from Linux](#cross-compile-from-linux)
  - [Native](#native)
- [Compile-time switches](#compile-time-switches)

## Dependencies

| Name          | Version     | Modules                                                  |
|---------------|-------------|----------------------------------------------------------|
| [Qt]          | >= 5.5.0    | concurrent, core, gui, network, opengl, svg, widget, xml |
| [GCC]/[MinGW] | >= 4.8      | C++11 enabled                                            |
| [toxcore]     | >= 0.2.10   | core, av                                                 |
| [FFmpeg]      | >= 2.6.0    | avformat, avdevice, avcodec, avutil, swscale             |
| [CMake]       | >= 2.8.11   |                                                          |
| [OpenAL Soft] | >= 1.16.0   |                                                          |
| [qrencode]    | >= 3.0.3    |                                                          |
| [sqlcipher]   | >= 3.2.0    |                                                          |
| [pkg-config]  | >= 0.28     |                                                          |
| [filteraudio] | >= 0.0.1    | optional dependency                                      |
| [snorenotify] | >= 0.7.0    | optional dependency                                      |

## Optional dependencies

They can be disabled/enabled by passing arguments to `cmake` command when
building qTox.

If they are missing, qTox is built without support for the functionality.

### Development dependencies

Dependencies needed to run tests / code formatting, etc. Disabled if
dependencies are missing.

| Name    | Version |
|---------|---------|
| [Check] | >= 0.9  |

### Spell checking support

| Name     | Version |
|----------|---------|
| [sonnet] | >= 5.45 |

Use `-DSPELL_CHECK=OFF` to disable it.

**Note:** Specified version was tested and works well. You can try to use older
version, but in this case you may have some errors (including a complete lack
of spell check).

### Linux

#### Auto-away support

| Name            | Version  |
|-----------------|----------|
| [libXScrnSaver] | >= 1.2   |
| [libX11]        | >= 1.6.0 |

Disabled if dependencies are missing during compilation.

#### Snorenotify desktop notification backend

Disabled by default

| Name              | Version   |
|-------------------|-----------|
| [snorenotify]     | >= 0.7.0  |

To enable: `-DDESKTOP_NOTIFICATIONS=True`


## Linux
### Simple install

Easy qTox install is provided for variety of distributions:

* [Arch](#arch)
* [Fedora](#fedora)
* [Gentoo](#gentoo)
* [Slackware](#slackware)

---

<a name="arch-easy" />

#### Arch

PKGBUILD is available in the `community` repo, to install:

```bash
pacman -S qtox
```

<a name="fedora-easy" />

#### Fedora

qTox is available in the [RPM Fusion](https://rpmfusion.org/) repo, to install:

```bash
dnf install qtox
```

<a name="gentoo-easy" />

#### Gentoo

qTox is available in Gentoo.

To install:

```bash
emerge qtox
```

<a name="opensuse-easy" />

#### openSUSE

qTox is available in openSUSE Factory.

To install in openSUSE 15.0 or newer:

```bash
zypper in qtox
```

To install in openSUSE 42.3:

```bash
zypper ar -f https://download.opensuse.org/repositories/server:/messaging/openSUSE_Leap_42.3 server:messaging
zypper in qtox
```

<a name="slackware-easy" />

#### Slackware

qTox SlackBuild and all of its dependencies can be found here:
http://slackbuilds.org/repository/14.2/network/qTox/

----

If your distribution is not listed, or you want / need to compile qTox, there
are provided instructions.


----

Most of the dependencies should be available through your package manager. You
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
cd /home/$USER/qTox
git clone https://github.com/qTox/qTox.git qTox
```

The following steps assumes that you cloned the repository at
`/home/$USER/qTox`.  If you decided to choose another location, replace
corresponding parts.


<a name="other-deps" />

### GCC, Qt, FFmpeg, OpenAL Soft and qrencode

<a name="arch-other-deps" />

#### Arch Linux

```bash
sudo pacman -S --needed base-devel qt5 openal libxss qrencode ffmpeg opus libvpx libsodium sqlcipher
```


<a name="debian-other-deps" />

#### Debian

```bash
sudo apt-get install \
    automake \
    autotools-dev \
    build-essential \
    check \
    checkinstall \
    cmake \
    ffmpeg \
    libavcodec-dev \
    libavdevice-dev \
    libexif-dev \
    libgdk-pixbuf2.0-dev \
    libgtk2.0-dev \
    libkdeui5 \
    libopenal-dev \
    libopus-dev \
    libqrencode-dev \
    libqt5opengl5-dev \
    libqt5svg5-dev \
    libsodium-dev \
    libsqlcipher-dev \
    libtool \
    libvpx-dev \
    libxss-dev \
    pkg-config \
    qrencode \
    qt5-default \
    qttools5-dev \
    qttools5-dev-tools \
    yasm
```

<a name="fedora-other-deps" />

#### Fedora

To install FFmpeg, the [RPM Fusion](https://rpmfusion.org/) repo is required.

```bash
sudo dnf group install "Development Tools" "C Development Tools and Libraries"
# (can also use):
# sudo dnf install @"Development Tools" @"C Development Tools and Libraries"
sudo dnf install \
    autoconf \
    automake \
    check \
    check-devel \
    ffmpeg-devel \
    gtk2-devel \
    kf5-sonnet \
    libexif-devel \
    libsodium-devel \
    libtool \
    libvpx-devel \
    libXScrnSaver-devel \
    openal-soft-devel \
    openssl-devel \
    opus-devel \
    qrencode-devel \
    qt5-devel \
    qt5-linguist \
    qt5-qtsvg \
    qt5-qtsvg-devel \
    qt-creator \
    qt-doc \
    qtsingleapplication-qt5 \
    sqlcipher \
    sqlcipher-devel
```

<a name="opensuse-other-deps" />

#### openSUSE

```bash
sudo zypper install \
    libexif-devel \
    libffmpeg-devel \
    libopus-devel \
    libQt5Concurrent-devel \
    libqt5-linguist \
    libQt5Network-devel \
    libQt5OpenGL-devel \
    libqt5-qtbase-common-devel \
    libqt5-qtsvg-devel \
    libQt5Xml-devel \
    libsodium-devel \
    libvpx-devel \
    libXScrnSaver-devel \
    openal-soft-devel \
    patterns-openSUSE-devel_basis \
    qrencode-devel \
    sqlcipher-devel \
    sonnet-devel \
    qt5-linguist-devel \
    libQt5Test-devel \
    ffmpeg-4-libavcodec-devel \
    ffmpeg-4-libavdevice-devel
```

<a name="slackware-other-deps" />

#### Slackware

List of all the toxcore dependencies and their SlackBuilds can be found
here: http://slackbuilds.org/repository/14.2/network/toxcore/

List of all the qTox dependencies and their SlackBuilds can be found here:
http://slackbuilds.org/repository/14.2/network/qTox/


<a name="ubuntu-other-deps" />

#### Ubuntu:

```bash
sudo apt-get install \
    build-essential \
    cmake \
    libavcodec-dev \
    libavdevice-dev \
    libavfilter-dev \
    libavutil-dev \
    libexif-dev \
    libgdk-pixbuf2.0-dev \
    libglib2.0-dev \
    libgtk2.0-dev \
    libkdeui5 \
    libopenal-dev \
    libopus-dev \
    libqrencode-dev \
    libqt5opengl5-dev \
    libqt5svg5-dev \
    libsodium-dev \
    libsqlcipher-dev \
    libswresample-dev \
    libswscale-dev \
    libvpx-dev \
    libxss-dev \
    qrencode \
    qt5-default \
    qttools5-dev-tools \
    qttools5-dev
```

### Compile toxcore

Normally you don't want to do that, `bootstrap.sh` will do it for you.

Provided that you have all required dependencies installed, you can simply run:

```bash
git clone https://github.com/toktok/c-toxcore.git toxcore
cd toxcore
git checkout v0.2.11
cmake . -DBOOTSTRAP_DAEMON=OFF
make -j$(nproc)
sudo make install

# we don't know what whether user runs 64 or 32 bits, and on some distros
# (Fedora, openSUSE) lib/ doesn't link to lib64/, so add both
echo '/usr/local/lib64/' | sudo tee -a /etc/ld.so.conf.d/locallib.conf
echo '/usr/local/lib/' | sudo tee -a /etc/ld.so.conf.d/locallib.conf
sudo ldconfig
```

### Compile qTox

**Make sure that all the dependencies are installed.**  If you experience
problems with compiling, it's most likely due to missing dependencies, so please
make sure that you did install *all of them*.

If you are compiling on Fedora 25, you must add libtoxcore to the
`PKG_CONFIG_PATH` environment variable manually:

```
# we don't know what whether user runs 64 or 32 bits, and on some distros
# (Fedora, openSUSE) lib/ doesn't link to lib64/, so add both
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib64/pkgconfig"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"
```

Run in qTox directory to compile:

```bash
cmake .
make -j$(nproc)
```

Now you can start compiled qTox with `./qtox`

Congratulations, you've compiled qTox `:)`


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


---

### Building packages

Alternately, qTox now has the experimental and probably-dodgy ability to package
itself (in `.deb` form natively, and `.rpm` form with
[alien](http://joeyh.name/code/alien/)).

After installing the required dependencies, run `bootstrap.sh` and then run the
`buildPackages.sh` script, found in the tools folder. It will automatically get
the packages necessary for building `.deb`s, so be prepared to type your
password for sudo.

---

### Security hardening with AppArmor

See [AppArmor] to enable confinement for increased security.

## BSD

<a name="freebsd-easy" />

#### FreeBSD

qTox is available as a binary package. To install the qTox package:

```bash
pkg install qTox
```

The qTox port is also available at ``net-im/qTox``. To build and install qTox
from sources using the port:

```bash
cd /usr/ports/net-im/qTox
make install clean
```

<a name="osx" />

## OS X

Supported OS X versions: >=10.8. (NOTE: only 10.13 is tested during CI)

Compiling qTox on OS X for development requires 2 tools:
[Xcode](https://developer.apple.com/xcode/) and [homebrew](https://brew.sh).

### Automated Script

You can now set up your OS X system to compile qTox automatically thanks to the
script in: `./osx/qTox-Mac-Deployer-ULTIMATE.sh`

This script can be run independently of the qTox repo and is all that's needed
to build from scratch on OS X.

To use this script you must launch terminal which can be found:
`Applications > Utilities > Terminal.app`

If you wish to lean more you can run `./qTox-Mac-Deployer-ULTIMATE.sh -h`

Note that the script will revert any non-committed changes to qTox repository
during the `update` phase.

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

Next, install
[toxcore](https://github.com/toktok/c-toxcore/blob/master/INSTALL.md#osx)

Then, clone qTox:

```bash
git clone https://github.com/qTox/qTox
```

Finally, copy all required files. Whenever you update your brew packages, you
may skip all of the above steps and simply run the following commands:

```bash
cd ./git/qTox
sudo bash bootstrap-osx.sh
```

#### Compiling

You can build qTox with Qt Creator
[seperate download](http://www.qt.io/download-open-source/#section-6) or
manually with cmake

With that; in your terminal you can compile qTox in the git dir:

```bash
cmake .
make
```

Or a cleaner method would be to:

```bash
cd ./git/dir/qTox
mkdir ./build
cd build
cmake ..
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

### Cross-compile from Linux

See [`windows/cross-compile`](windows/cross-compile).

### Native

#### Qt

Download the Qt online installer for Windows from
[qt.io](https://www.qt.io/download-open-source/). While installation you have
to assemble your Qt toolchain. Take the most recent version of Qt compiled with
MinGW. Although the installer provides its own bundled MinGW compiler toolchain
its recommend installing it separately because Qt is missing MSYS which is
needed to compile and install OpenAL. Thus you can - if needed - deselect the
tab `Tools`. The following steps assume that Qt is installed at `C:\Qt`. If you
decided to choose another location, replace corresponding parts.

#### MinGW

Download the MinGW installer for Windows from
[sourceforge.net](http://sourceforge.net/projects/mingw/files/Installer/). Make
sure to install MSYS (a set of Unix tools for Windows). The following steps
assume that MinGW is installed at `C:\MinGW`. If you decided to choose another
location, replace corresponding parts. Select `mingw-developer-toolkit`,
`mingw32-base`, `mingw32-gcc-g++`, `msys-base` and `mingw32-pthreads-w32`
packages using MinGW Installation Manager (`mingw-get.exe`). Check that the
version of MinGW, corresponds to the version of the QT component!

#### Wget

Download the Wget installer for Windows from
http://gnuwin32.sourceforge.net/packages/wget.htm. Install them. The following
steps assume that Wget is installed at `C:\Program Files (x86)\GnuWin32\`. If you
decided to choose another location, replace corresponding parts.

#### UnZip

Download the UnZip installer for Windows from
http://gnuwin32.sourceforge.net/packages/unzip.htm. Install it. The following
steps assume that UnZip is installed at `C:\Program Files (x86)\GnuWin32\`. If you
decided to choose another location, replace corresponding parts.

#### Setting up Path

Add MinGW/MSYS/CMake binaries to the system path to make them globally
accessible. Open `Control Panel` -> `System and Security` -> `System` ->
`Advanced system settings` -> `Environment Variables...` (or run `sysdm.cpl`
select tab `Advanced system settings` -> button `Environment Variables`). In the
second box search for the `PATH` variable and press `Edit...`. The input box
`Variable value:` should already contain some directories. Each directory is
separated with a semicolon. Extend the input box by adding
`;C:\MinGW\bin;C:\MinGW\msys\1.0\bin;C:\Program Files (x86)\CMake 2.8\bin;C:\Program Files (x86)\GnuWin32\bin`.
The very first semicolon must only be added if it is missing. CMake may be added
by installer automatically. Make sure that paths containing alternative `sh`,
`bash` implementations such as `C:\Program Files\OpenSSH\bin` are at the end of
`PATH` or build may fail.

#### Cloning the Repository

Clone the repository (https://github.com/qTox/qTox.git) with your preferred Git
client. [SmartGit](http://www.syntevo.com/smartgit/) or
[TorteiseGit](https://tortoisegit.org) are both very nice for this task
(you may need to add `git.exe` to your `PATH` system variable). The
following steps assume that you cloned the repository at `C:\qTox`. If you
decided to choose another location, replace corresponding parts.

#### Getting dependencies

Run `bootstrap.bat` in the previously cloned `C:\qTox` repository. The script will
download the other necessary dependencies, compile them and put them into their
appropriate directories.

Note that there have been detections of false positives by some anti virus software
in the past within some of the libraries used. Please refer to the wiki page
[problematic antiviruses](https://github.com/qTox/qTox/wiki/Problematic-antiviruses)
for more information if you run into troubles on that front.

## Compile-time switches

They are passed as an argument to `cmake` command. E.g. with a switch `SWITCH`
that has value `YES` it would be passed to `cmake` in a following manner:

```bash
cmake -DSWITCH=yes
```

Switches:

- `SMILEYS`, values:
  - if not defined or an unsupported value is passed, all emoticon packs are
    included
  - `DISABLED` – don't include any emoticon packs, custom ones are still loaded
  - `MIN` – minimal support for emoticons, only a single emoticon pack is
    included


[AppArmor]: /security/apparmor/README.md
[Atk]: https://wiki.gnome.org/Accessibility
[Cairo]: https://www.cairographics.org/
[Check]: https://libcheck.github.io/check/
[CMake]: https://cmake.org/
[DBus Menu]: https://launchpad.net/libdbusmenu
[FFmpeg]: https://www.ffmpeg.org/
[GCC]: https://gcc.gnu.org/
[libX11]: https://www.x.org/wiki/
[libXScrnSaver]: https://www.x.org/wiki/Releases/ModuleVersions/
[MinGW]: http://www.mingw.org/
[OpenAL Soft]: http://kcat.strangesoft.net/openal.html
[Pango]: http://www.pango.org/
[pkg-config]: https://www.freedesktop.org/wiki/Software/pkg-config/
[qrencode]: https://fukuchi.org/works/qrencode/
[Qt]: https://www.qt.io/
[toxcore]: https://github.com/TokTok/c-toxcore/
[filteraudio]: https://github.com/irungentoo/filter_audio
[sonnet]: https://github.com/KDE/sonnet
[snorenotify]: https://techbase.kde.org/Projects/Snorenotify
