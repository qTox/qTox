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

Debian:
```bash
sudo apt-get install git
```

Fedora:
```bash
yum install git
```

Ubuntu:
```bash
sudo apt-get install git
```

Afterwards open a new Terminal, change to a directory of your choice and clone the repository:
```bash
cd /home/user/qTox
git clone https://github.com/tux3/qTox.git qTox
```

The following steps assumes that you cloned the repository at "/home/user/qTox". If you decided to choose another location, replace corresponding parts.

###GCC, Qt, OpenCV and OpanAL Soft

Arch Linux:
```bash
sudo pacman -S --needed base-devel qt5 opencv openal libxss
```

Debian:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default libqt5opengl5-dev libqt5svg5-dev libopenal-dev libopencv-dev libxss-dev
```

Fedora:
```bash
yum groupinstall "Development Tools"
yum install qt-devel qt-doc qt-creator qt5-qtsvg opencv-devel openal-soft-devel libXScrnSaver-devel
```

Slackware:

You can grab slackbuilds of the needed dependencies here:

http://slackbuilds.org/repository/14.1/libraries/OpenAL/

http://slackbuilds.org/repository/14.1/libraries/qt5/

http://slackbuilds.org/repository/14.1/libraries/opencv/

Ubuntu:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default qttools5-dev-tools libopenal-dev libopencv-dev libxss-dev
```

###Tox Core

First of all install the dependencies of Tox Core.

Arch Linux:
```bash
sudo pacman -S --needed opus vpx
```

```
Debian:
```bash
sudo apt-get install libtool autotools-dev automake checkinstall check libopus-dev libvpx-dev
```

Fedora:
```bash
yum install libtool autoconf automake check check-devel
```

Ubuntu:
```bash
sudo apt-get install libtool autotools-dev automake checkinstall check libopus-dev libvpx-dev
```

Now you can either follow the instructions at https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#unix or use the "bootstrap.sh" script located at "/home/user/qTox".
The script will automatically download and install Tox Core and libsodium to "/home/user/qTox/libs":
```bash
cd /home/user/qTox
./bootstrap.sh # use -h or --help for more information
```

###filter_audio
You also need to install filter_audio library separately if you did not run ``./bootstrap.sh``.
```bash
./install_libfilteraudio.sh
```

After all the dependencies are thus reeady to go, compiling should be as simple as 
```bash
qmake
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

###OSX Easy Install

Since https://github.com/ReDetection/homebrew-qtox you can easily install qtox with homebrew 
```bash
brew install --HEAD ReDetection/qtox/qtox
```


###OSX Full Install Guide

This guide is intended for people who wish to use an existing or new ProjectTox-Core installation separate to the bundled installation with qTox, if you do not wish to use a separate installation you can skip to the section titled 'Final Steps'.

Installation on OSX, isn't quite straight forward, here is a quick guide on how to install;

Note that qTox now requires OpenCV and OpenAL for video and audio.

The first thing you need to do is install ProjectTox-Core with a/v support. Refer to the INSTALL guide in the PrjectTox-Core github repo.

Next you need to download QtTools (http://qt-project.org/downloads), at the time of writing this is at version .3.0.
Make sure you deselect all the unnecessary components from the 5.3 checkbox (iOS/Android libs) otherwise you will end up with a very large download.

Once that is installed you will most likely need to set the path for qmake. To do this, open up terminal and paste in the following;

```bash
export PATH=/location/to/qmake/binary:$PATH
```

For myself, the qmake binary was located in /Users/mouseym/Qt/5.3/clang_64/bin/.

This is not a permanent change, it will revert when you close the terminal window, to add it permanently you will need to add echo the above line to your .profile/.bash_profile.

Once this is installed, do the following;

```bash
git clone https://github.com/tux3/qTox
cd toxgui
qmake
```

Now, we need to create a symlink to /usr/local/lib/ and /usr/local/include/
```
mkdir -p $HOME/qTox/libs
sudo ln -s /usr/local/lib $HOME/qTox/libs/lib
sudo ln -s /usr/local/include  $HOME/qTox/libs/include
```
####Final Steps

The final step is to run 
```bash
make
``` 
in the qTox directory, or if you are using the bundled tox core installation, you can use 
```bash
./bootstrap.sh
make
```
Assuming all went well you should now have a qTox.app file within the directory. Double click and it should open!


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

###Setting up Path

Add MinGW/MSYS/CMake binaries to the system path to make them globally accessible. 
Open Control Panel -> System and Security -> System -> Advanced system settings -> Environment Variables...
In the second box search for the PATH variable and press Edit...
The input box "Variable value:" should already contain some directories. Each directory is separated with a semicolon.
Extend the input box by adding ";C:\MinGW\bin;C:\MinGW\msys\1.0\bin;C:\Program Files (x86)\CMake 2.8\bin".
The very first semicolon must only be added if it is missing. CMake may be added by installer automatically.

###Cloning the Repository

Clone the repository (https://github.com/tux3/qTox.git) with your preferred  Git client. [SmartGit](http://www.syntevo.com/smartgit/) is very nice for this task.
The following steps assume that you cloned the repository at "C:\qTox". If you decided to choose another location, replace corresponding parts.

### Getting dependencies
Run bootstrap.bat in cloned C:\qTox directory
Script will download rest of dependencies compile them and put to appropriate directories.
