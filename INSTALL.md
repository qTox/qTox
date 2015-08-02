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
| toxcore      | most recent | core, av                                          |
| FFmpeg       | >= 2.6.0    | avformat, avdevice, avcodec, avutil, swscale      |
| OpenAL Soft  | >= 1.16.0   |                                                   |
| filter_audio | most recent |                                                   |
| qrencode     | >= 3.0.3    |                                                   |

Note to Fedora users: check qt5 version before building default is 4.8 on fedora 21 / 22, everything up until qmake-qt5 will build fine but then  qmake-qt5 will freak out.

<a name="linux" />
##Linux
###Simple install
Easy qTox install is provided for variety of distributions:

* [Arch](#arch)
* [Debian, Mint, Ubuntu, etc](#debian)
* [Gentoo](#gentoo)
* [Slackware](#slackware)


#### Arch

**Please note that installing toxcore/qTox from AUR is not supported**, although installing other dependencies, provided that they met requirements, should be fine, unless you are installing cryptography library from AUR, which should rise red flags by itself…

That being said, there are supported PKGBUILDs at https://github.com/Tox/arch-repo-tox


<a name="debian" />
#### Debian, Mint, Ubuntu, etc

Use this script to add repository:
```bash
sudo sh -c 'echo "deb https://pkg.tox.chat/ nightly main" > /etc/apt/sources.list.d/tox.list'
wget -qO - https://pkg.tox.chat/pubkey.gpg | sudo apt-key add -
sudo apt-get install apt-transport-https
sudo apt-get update -qq
echo "qTox Repository Installed."
```


#### Gentoo

qTox ebuild is available in ``tox-overlay``. To add it and install qTox you will need to have installed ``layman``:
```bash
emerge layman
```

After that, add overlay and install qTox:
```bash
layman -f
layman -a tox-overlay
emerge qtox
```


#### Slackware

qTox SlackBuild and all of its dependencies can be found here: http://slackbuilds.org/repository/14.1/network/qTox/

----

If your distribution is not listed, or you want/need to compile qTox, there are provided instructions.


----

Most of the dependencies should be available through your package manger. You may either follow the directions below, or simply run `./simple_make.sh` after cloning this repository, which will attempt to automatically download dependencies followed by compilation.


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

###GCC, Qt, FFmpeg, OpanAL Soft and QRCode

Arch Linux:
```bash
sudo pacman -S --needed base-devel qt5 openal libxss qrencode ffmpeg
```

Debian <10 / Ubuntu <15.04:
**Note that ffmpeg is not included in those distribution version(!).**
```bash
sudo apt-get install build-essential qt5-qmake qt5-default qttools5-dev-tools libqt5opengl5-dev libqt5svg5-dev libopenal-dev libxss-dev qrencode libqrencode-dev libglib2.0-dev libgdk-pixbuf2.0-dev libgtk2.0-dev
```

Debian >=10 / Ubuntu >=15.04:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default qttools5-dev-tools libqt5opengl5-dev libqt5svg5-dev libopenal-dev libxss-dev qrencode libqrencode-dev libavutil-ffmpeg-dev libswresample-ffmpeg-dev libavcodec-ffmpeg-dev libswscale-ffmpeg-dev libavfilter-ffmpeg-dev libavdevice-ffmpeg-dev libglib2.0-dev libgdk-pixbuf2.0-dev libgtk2.0-dev
```


Fedora:
```bash
sudo dnf group install "Development Tools"
sudo dnf install qt-devel qt-doc qt-creator qt5-qtsvg qt5-qtsvg-devel openal-soft-devel libXScrnSaver-devel qrencode-devel
```

openSUSE:
**Note that ffmpeg is not included in the default repositories, you have to add the packman repository.**
```bash
sudo zypper install patterns-openSUSE-devel_basis libqt5-qtbase-common-devel libqt5-qtsvg-devel libqt5-linguist libQt5Network-devel libQt5OpenGL-devel libQt5Concurrent-devel libQt5Xml-devel libQt5Sql-devel openal-soft-devel qrencode-devel libXScrnSaver-devel libQt5Sql5-sqlite 
```

Slackware:

List of all the ``qTox`` dependencies and their SlackBuilds can be found here: http://slackbuilds.org/repository/14.1/network/qTox/


### toxcore

First of all install the dependencies of toxcore.

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

List of all the ``toxcore`` dependencies and their SlackBuilds can be found here: http://slackbuilds.org/repository/14.1/network/toxcore/

----
###filter_audio
You also need to install filter_audio library separately if you did not run ``./bootstrap.sh``.

This step is  best done before compiling Toxcore if not  using package manager. This also follows the flow of the homebrew instructions and reduces the likelihood of  later finding errors for  libavcodec compiling toxcore.

```bash
git clone https://github.com/irungentoo/filter_audio
cd filter_audio
make
sudo make install

Now you can either follow the instructions at https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#unix
or use the "bootstrap.sh" script located at "/home/user/qTox".
The script will automatically download and install toxcore and libfilteraudio:

```bash
cd /home/user/qTox
./bootstrap.sh # use -h or --help for more information


if  using  /usr/local/bin for final build make sure to follow advise here: https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#unix, regarding  sudo ldconfig
```


###Compiling
**Make sure that all the dependencies are installed.**  
Now go to `/home/user/qTox/qTox` (or where you cloned) and simply run :
```bash
qmake
make
```

for openSUSE / Fedora you have to use:
```bash
qmake-qt5
make
```

(Debian / Ubuntu / Mint)
If the compiling process stops with a missing dependency like: `... libswscale/swscale.h missing` try:
```
apt-file search libswscale/swscale.h
```
And install the package that provides the missing file.
Start make again. Repeat if nessary until all dependencies are installed.  If you can, please note down all additional dependencies you had to install that aren't listed here, and let us know what is missing `;)`



###Building packages

Alternately, qTox now has the experimental and probably-dodgy ability to package itself (in .deb
form natively, and .rpm form with <a href="http://joeyh.name/code/alien/">alien</a>).

After installing the required dependencies, run `bootstrap.sh` and then run the
`buildPackages.sh` script, found in the tools folder. It will automatically get the
packages necessary for building .debs, so be prepared to type your password for sudo.


<a name="osx" />
##OS X
Compiling qTox on OS X for development requires 3 tools, [Xcode](https://developer.apple.com/xcode/) and [Qt 5.4+](http://www.qt.io/qt5-4/), and [homebrew](http://brew.sh).

###Required Libraries

First, let's install the dependencies available via brew.
```bash
brew install git ffmpeg qrencode
```

Next, install [filter_audio](https://github.com/irungentoo/filter_audio) (you may delete the directory it creates afterwards):
```bash
git clone https://github.com/irungentoo/filter_audio.git
cd filter_audio
sudo make install
cd ../
```

Then, clone qTox:
```bash
git clone https://github.com/tux3/qTox``
```

Finally, copy all required files. Whenever you update your brew packages, you may skip all of the above steps and simply run the following commands:
```bash
cd qTox
sudo bash bootstrap-osx.sh
```

###Compiling

Either open Qt creator and hit build or run ```qmake && make``` in your qTox folder and it'll just work™.

Note that if you use the CLI to build you'll need to add Qt5's bins to your path.
```bash
export PATH=$PATH:~/Qt/5.4/clang_64/bin/
```

###Fixing things up

The bad news is that Qt breaks our linker paths so we need to fix those. First cd in to your qtox.app directory, if you used Qt Creator it's in ```~/build-qtox-Desktop_Qt_5_4_1_clang_64bit-Release``` most likely, otherwise it's in your qTox folder.

Install qTox so we can copy its libraries and shove the following in a script somewhere:

```bash
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
```bash
qtox.app/Contents/MacOS/qtox
```
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
Check that the version of MinGW, corresponds to the version of the QT component!

###WGet
Download the WGet installer for Windows from(
http://gnuwin32.sourceforge.net/packages/wget.htm).
Install them. The following steps assume that WGet is installed at "C:\Program Files\GnuWin32\". If you decided to choose another location, replace corresponding parts.

###Setting up Path

Add MinGW/MSYS/CMake binaries to the system path to make them globally accessible. 
Open Control Panel -> System and Security -> System -> Advanced system settings -> Environment Variables...(or run "sysdm.cpl" select tab "Advanced system settings" -> button "Environment Variables")
In the second box search for the PATH variable and press Edit...
The input box "Variable value:" should already contain some directories. Each directory is separated with a semicolon.
Extend the input box by adding ";C:\MinGW\bin;C:\MinGW\msys\1.0\bin;C:\Program Files (x86)\CMake 2.8\bin;C:\Program Files\GnuWin32\bin".
The very first semicolon must only be added if it is missing. CMake may be added by installer automatically.

###Cloning the Repository

Clone the repository (https://github.com/tux3/qTox.git) with your preferred  Git client. [SmartGit](http://www.syntevo.com/smartgit/) is very nice for this task (you may need to add the path to the git.exe system variable Path).
The following steps assume that you cloned the repository at "C:\qTox". If you decided to choose another location, replace corresponding parts.

### Getting dependencies
Run bootstrap.bat in cloned C:\qTox directory
Script will download rest of dependencies compile them and put to appropriate directories.
