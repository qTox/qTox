#Install Instructions
- [Dependencies](#dependencies)
- [Linux](#linux)
- [OS X](#osx)
- [Windows](#windows)

<a name="dependencies" />
##Dependencies

| Name         | Version     | Modules                                           |
|--------------|-------------|-------------------------------------------------- |
| Qt           | >= 5.2.0    | core, gui, network, opengl, sql, svg, widget, xml |
| GCC/MinGW    | >= 4.8      | C++11 enabled                                     |
| Tox Core     | most recent | core, av                                          |
| OpenCV       | >= 2.4.9    | core, highgui, imgproc                            |
| OpenAL Soft  | >= 1.16.0   |                                                   |
| filter_audio | most recent |                                                   |
| qrencode     | >= 3.0.3    |                                                   |

<a name="linux" />
##Linux
###Simple install
Easy qTox install is provided for variety of distributions:
https://wiki.tox.im/Binaries#Apt.2FAptitude_.28Debian.2C_Ubuntu.2C_Mint.2C_etc..29

If your distribution is not listed, or you want/need to compile qTox, there are provided instructions.

**Please note that installing toxcore/qTox from AUR is not supported**, although installing other dependencies, provided that they met requirements, should be fine, unless you are installing cryptography library from AUR, which should rise red flags by itself…

----

Most of the dependencies should be available through your package manger. You may either follow the directions below, or simply run `./simple_make.sh` after cloning, which will attempt to automatically download dependencies followed by compilation.


###Cloning the Repository
In order to clone the qTox repository you need Git.

Arch Linux:
```bash
sudo pacman -S --needed git
```

Debian / Ubuntu:
```bash
sudo apt-get install git
```

Fedora: (yum is now officially deprecated by dnf using yum will redirect to dnf on Fedora 21 and fail on future versions)
```bash
sudo dnf install git
```

openSUSE:
```bash
sudo zypper install git
```


Afterwards open a new Terminal, change to a directory of your choice and clone the repository:
```bash
cd /home/user/qTox
git clone https://github.com/tux3/qTox.git qTox
```

The following steps assumes that you cloned the repository at "/home/user/qTox". If you decided to choose another location, replace corresponding parts.

###GCC, Qt, OpenCV, OpanAL Soft and QRCode

Arch Linux:
```bash
sudo pacman -S --needed base-devel qt5 opencv openal libxss qrencode
```

Debian / Ubuntu:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default qttools5-dev-tools libqt5opengl5-dev libqt5svg5-dev libopenal-dev libopencv-dev libxss-dev qrencode libqrencode-dev
```

Fedora:
```bash
dnf group install "Development Tools"
dnf install qt-devel qt-doc qt-creator qt5-qtsvg opencv-devel openal-soft-devel libXScrnSaver-devel qrencode-devel
```

openSUSE:

If you are running openSUSE 13.2 you have to add the following repository to be able to install opencv-qt5.

WARNING: This may break other applications that are depending on opencv.

```bash
sudo zypper ar http://download.opensuse.org/repositories/KDE:/Extra/openSUSE_13.2/ 'openSUSE BuildService - KDE:Extra'
```

With openSUSE Tumbleweed you can continue here:
```bash
sudo zypper install patterns-openSUSE-devel_basis libqt5-qtbase-common-devel libqt5-qtsvg-devel libqt5-linguist libQt5Network-devel libQt5OpenGL-devel libQt5Concurrent-devel libQt5Xml-devel libQt5Sql-devel openal-soft-devel qrencode-devel libXScrnSaver-devel libQt5Sql5-sqlite opencv-qt5-devel 
```

Slackware:
```bash
You can grab SlackBuilds of the needed dependencies here:

http://slackbuilds.org/repository/14.1/libraries/OpenAL/
http://slackbuilds.org/repository/14.1/libraries/qt5/
http://slackbuilds.org/repository/14.1/libraries/opencv/
http://slackbuilds.org/repository/14.1/graphics/qrencode/
```

###Tox Core

First of all install the dependencies of Tox Core.

Arch Linux:
```bash
sudo pacman -S --needed opus libvpx libsodium
```

Debian / Ubuntu:
```bash
sudo apt-get install libtool autotools-dev automake checkinstall check libopus-dev libvpx-dev libsodium-dev
```

Fedora:
```bash
sudo dnf install libtool autoconf automake check check-devel libsodium-devel
```

openSUSE:
```bash
sudo zypper install libsodium-devel libvpx-devel libopus-devel patterns-openSUSE-devel_basis
```

Slackware:
```bash
You can grab SlackBuilds of the needed dependencies here:

http://slackbuilds.org/repository/14.1/audio/opus/
http://slackbuilds.org/repository/14.1/libraries/libvpx/
http://slackbuilds.org/repository/14.1/libraries/libsodium/
```

Now you can either follow the instructions at https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#unix or use the "bootstrap.sh" script located at "/home/user/qTox".
The script will automatically download and install Tox Core and libfilteraudio:
```bash
cd /home/user/qTox
./bootstrap.sh # use -h or --help for more information
```

###filter_audio
You also need to install filter_audio library separately if you did not run ``./bootstrap.sh``.
```bash
git clone https://github.com/irungentoo/filter_audio
cd filter_audio
make
sudo make install
```

After all the dependencies are installed, compiling should be as simple as:
```bash
qmake
make
```

for openSUSE you have to use:
```bash
qmake-qt5
make
```

###Building packages

Alternately, qTox now has the experimental and probably-dodgy ability to package itself (in .deb
form natively, and .rpm form with <a href="http://joeyh.name/code/alien/">alien</a>).

After installing the required dependencies, run `bootstrap.sh` and then run the
`buildPackages.sh` script, found in the tools folder. It will automatically get the
packages necessary for building .debs, so be prepared to type your password for sudo.


<a name="osx" />
##OS X
Please be aware that if you've tried an earlier version of this set of instructions you may have 
installed broken libraries and packages in the proces. Please delete them before continuing.

Also, if you want to use qTox and are an end user download it by clicking the download button on tox.im, 
as the copy you'll make by following this guide is only suitable for testing.

Compiling qTox on OS X for development requires 3 tools, [Xcode](https://developer.apple.com/xcode/) and [Qt 5.4+](http://www.qt.io/qt5-4/), and [homebrew](http://brew.sh).

###Required tools

First, let's install the dependencies
* ```brew install git wget```
* ``git clone https://github.com/tux3/qTox``
* ```cd qTox```

###Libraries required to compile

Now we are in the qTox folder and need our library dependencies to actually build it.

We've taken the time to prepare them automatically with our CI system so if you ever have issues redownload them.

* ```wget https://jenkins.libtoxcore.so/job/qTox%20OS%20X/lastSuccessfulBuild/artifact/dep.zip```
* ```unzip dep.zip```

If you do not want to download our binaries, you must compile [opencv2](http://opencv.org), [toxcore](https://github.com/irungentoo/toxcore), [opus](https://www.opus-codec.org), [vpx](http://www.webmproject.org/tools/), [filteraudio](https://github.com/irungentoo/filter_audio), and our fork of [openal](https://github.com/irungentoo/openal-soft-tox) yourself with the prefix to the libs folder.

Please be aware that no one has ever successfully got this working outside of on our CI system, but we encourage you to try and provide instructions on how you did so if you do.

Please be aware that you shouldn't do this on your main Mac, as it's fairly hard to successfully do this without ruining a bunch of things in the process.

Everything from opencv2 to filter_audio has now been installed in this library and is ready to go.

###Compiling

Either open Qt creator and hit build or run qmake && make in your qTox folder and it'll just work™

Note that if you use the CLI to build you'll need to add Qt5's bins to your path.
```export PATH=$PATH:~/Qt/5.4/clang_64/bin/```

###Fixing things up

The bad news is that Qt breaks our linker paths so we need to fix those.
First cd in to your qtox.app directory, if you used Qt Creator it's in ```~/build-qtox-Desktop_Qt_5_4_1_clang_64bit-Release``` most likely, otherwise it's in your qTox folder.

Install qTox so we can copy its libraries and shove the following in a script somewhere:

```
~macdeployqt qtox.app
cp -r /Applications/qtox.app qtox_old.app
cp qtox.app/Contents/MacOS/qtox qtox_old.app/Contents/MacOS/qtox
rm -rf qtox.app
mv qtox_old.app qtox.app
```
* Give it a name like ~/deploy.qtox.sh
* cd in to the folder with qtox.app
* run ```bash ~/deploy.qtox.sh```

###Running qTox
You've got 2 choices, either click on the qTox app that suddenly exists, or do the following:
* ``qtox.app/Contents/MacOS/qtox`` 
* Enjoy the snazzy CLI output as your friends and family congratulate you on becoming a hacker

<a name="windows" />
##Windows

###Qt

Download the Qt online installer for Windows from [qt-project.org](http://qt-project.org/downloads).
While installation you have to assemble your Qt toolchain. Take the most recent version of Qt compiled with MinGW.
Although the installer provides its own bundled MinGW compiler toolchain its recommend installing it separately because Qt is missing MSYS which is needed to compile and install OpenCV and OpenAL. Thus you can - if needed - deselect the tab "Tools".
The following steps assume that Qt is installed at "C:\Qt". If you decided to choose another location, replace corresponding parts.

###MinGW

Download the MinGW installer for Windows from [sourceforge.net](http://sourceforge.net/projects/mingw/files/Installer/).
Make sure to install MSYS (a set of Unix tools for Windows).
The following steps assume that MinGW is installed at "C:\MinGW". If you decided to choose another location, replace corresponding parts.

###qrencode
Download the qrencode from http://fukuchi.org/works/qrencode/ or direct from https://code.google.com/p/qrencode-win32/source/checkout ,
build project "..\qrencode-win32\vc8\qrcodelib\", you must copy files from release in: "qrcodelib.dll" to \qTox\libs\bin\qrcodelib.dll";
"qrencode.h" to \qTox\libs\include\qrencode.h"; "qrcodelib.lib" to "\qTox\libs\lib\qrencode.lib" with rename!!!

###Setting up Path

Add MinGW/MSYS/CMake binaries to the system path to make them globally accessible. 
Open Control Panel -> System and Security -> System -> Advanced system settings -> Environment Variables...
In the second box search for the PATH variable and press Edit...
The input box "Variable value:" should already contain some directories. Each directory is separated with a semicolon.
Extend the input box by adding ";C:\MinGW\bin;C:\MinGW\msys\1.0\bin;C:\Program Files (x86)\CMake 2.8\bin".
The very first semicolon must only be added if it is missing. CMake may be added by installer automatically.

###Cloning the Repository

Clone the repository (https://github.com/tux3/qTox.git) with your preferred  Git client. [SmartGit](http://www.syntevo.com/smartgit/) is very nice for this task (you may need to add the path to the git.exe system variable Path).
The following steps assume that you cloned the repository at "C:\qTox". If you decided to choose another location, replace corresponding parts.

### Getting dependencies
Run bootstrap.bat in cloned C:\qTox directory
Script will download rest of dependencies compile them and put to appropriate directories.
