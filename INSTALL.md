##Dependencies

| Name         | Version     | Modules                         |
|--------------|-------------|-------------------------------- |
| Qt           | >= 5.2.0    | core, gui, network, widget, xml |
| GCC/MinGW    | >= 4.6      | C++11 enabled                   |
| Tox Core     | most recent | core, av                        |
| OpenCV       | >= 2.4.9    | core, highgui                   |
| OpenAL Soft  | >= 1.16.0   |                                 |

##Windows

###Qt

Download the Qt online installer for Windows from [qt-project.org](http://qt-project.org/downloads).
While installation you have to assemble your Qt toolchain. Take the most recent version of Qt compiled with MinGW.
Although the installer provides its own bundled MinGW compiler Toolchain its recommend installing it separately because Qt is missing MSYS which is needed to compile and install OpenCV and OpenAL. Thus you can - if needed - deselect the tab "Tools".
The following steps assume that Qt is installed at "C:\Qt". If you decided to choose another location, replace corresponding parts.

###MinGW

Download the MinGW installer for Windows from [sourceforge.net](http://sourceforge.net/projects/mingw/files/Installer/).
Make sure to install MSYS (a set of Unix tools for Windows).
The following steps assume that MinGW is installed at "C:\MinGW". If you decided to choose another location, replace corresponding parts.

###Setting up Path

Add MinGW/MSYS binaries to the system path to make them globally accessible. 
Open Control Panel -> System and Security -> System -> Advanced system settings -> Environment Variables...
In the second box search for the PATH variable and press Edit...
The input box "Variable value:" should already contain some directories. Each directory is separated with a semicolon.
Extend the input box by adding ";C:\MinGW\bin;C:\MinGW\msys\1.0\bin". The very first semicolon must only be added if it is missing.

###Cloning the Repository

Clone the repository (https://github.com/tux3/qTox.git) with your preferred  Git client. [SmartGit](http://www.syntevo.com/smartgit/) is very nice for this task.
The following steps assume that you cloned the repository at "C:\qTox". If you decided to choose another location, replace corresponding parts.

###Tox Core

[jenkins.libtoxcore.so](http://jenkins.libtoxcore.so/job/libtoxcore-win32-i686/lastSuccessfulBuild/artifact/libtoxcore-win32-i686.zip) 
provides a prebuild package of Tox Core. Download this package and extract its content to "C:\qTox\libs". You may have to create the directory "libs".
If you prefer to compile Tox Core on your own follow the instructions at https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#windows

###OpenCV

Unfortunately there are no prebuild packages for OpenCV compiled with MinGW. Thus, you have to create your own.
First of all download and install the most recent version of CMake from
[cmake.org](http://www.cmake.org/cmake/resources/software.html).
Afterwards download the source archive of OpenCV from [http://opencv.org](http://opencv.org/downloads.html). It's recommended to download the Linux/Mac package because the Windows package is bloated with useless binaries. The Linux/Mac package only contains the sources and works perfectly on Windows.
Extract the content of the source archive to "C:\qTox\libs". Furthermore, create a new directory named "opencv-build" in "C:\qTox\libs". 
Now you should have the two directories "opencv-x.y.z" where x.y.z is the version of OpenCV and "opencv-build" inside your "C:\qTox\libs" directory.
Run CMake (cmake-gui) and set up the input boxes "Where is the source code:" and "Where to build the binaries" with "C:\qTox\libs\opencv-x.y.z" and "C:\qTox\libs\opencv-build". Press configure and choose "MSYS Makefiles" in the drop down menu and "Use default native compilers". Press "Finish" to start configuration. TODO

###OpenAL Soft



##Linux
Most of the dependencies should be available through your package manger.

###Cloning the Repository
In order to clone the qTox repository you need Git.

Debian:
```bash
sudo apt-get install git
```

Ubuntu:
```bash
sudo apt-get install git
```

Arch Linux:
```bash
sudo pacman -S git
```

Fedora:
```bash
yum install git
```

Afterwards open a new Terminal, change to a directory of your choice and clone the repository:
```bash
cd /home/user/qTox
git clone https://github.com/tux3/qTox.git qTox
```

The following steps assumes that you cloned the repository at "/home/user/qTox". If you decided to choose another location, replace corresponding parts.

###GCC, Qt, OpenCV and OpanAL Soft

Debian:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default libopenal-dev libopencv-dev
```

Ubuntu:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default libopenal-dev libopencv-dev
```

Arch Linux:
```bash
sudo pacman -S base-devel qt5 opencv openal
```

Fedora:
```bash
yum groupinstall "Development Tools"
yum install qt-devel qt-doc qt-creator opencv-devel openal-soft-devel
```

###Tox Core

First of all install the dependencies of Tox Core.

Debian:
```bash
sudo apt-get install libtool autotools-dev automake checkinstall check yasm libopus-dev libvpx-dev
```

Ubuntu:
```bash
sudo apt-get install libtool autotools-dev automake checkinstall check yasm libopus-dev libvpx-dev
```

Arch Linux: (Arch Linux provides the package "tox-git" in AUR)
```bash
sudo pacman -S yasm opus vpx
```

Fedora:
```bash
yum install libtool autoconf automake check check-devel
```

Now you can either follow the instructions at https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#unix or use the "bootstrap.sh" script located at "/home/user/qTox".
The script will automatically download and install Tox Core and libsodium to "/home/user/qTox/libs":
```bash
cd /home/user/qTox
./bootstrap.sh # use -h or --help for more information
```
