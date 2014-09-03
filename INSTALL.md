###Dependencies

| Name         | Version     | Modules                         |
|--------------|-------------|-------------------------------- |
| Qt           | >= 5.2.0    | core, gui, network, widget, xml |
| GCC/MinGW    | >= 4.6      | C++11 enabled                   |
| Tox Core     | most recent | core, av                        |
| OpenCV       | >= 2.4.9    | core, highgui                   |
| OpenAL Soft  | >= 1.16.0   |                                 |

## Windows

### Qt

Download the Qt online installer for Windows from [qt-project.org](http://qt-project.org/downloads).
While installation you have to assemble your Qt toolchain. Take the most recent version of Qt compiled with MinGW. 
Select "Tools" to install the MinGW compiler package itself alongside Qt. By doing this you don't have to bother with installing one manually.
The following steps assumes that Qt is installed at "C:\Qt". If you decided to choose another location, replace corresponding parts.

### MinGW

If you installed MinGW alongside Qt you can skip this step.

Download the MinGW installer for Windows from [sourceforge.net](http://sourceforge.net/projects/mingw/files/Installer/).
Make sure to install MSYS (a minimalistic set of Unix tools for Windows) and TODO.

### System Path

For some tasks it might be useful to use the Windows terminal. Thus you should add Qt/MinGW binaries to the system path to make them globally accessible. 
Open Control Panel -> System -> Advanced Settings -> .... TODO

### Cloning the Repository

Clone the repository (https://github.com/tux3/qTox.git) with your prefered Git client. [SmartGit](http://www.syntevo.com/smartgit/) is very nice for this task.
The following steps assumes that you cloned the repository at "C:\qTox". If you decided to choose another location, replace corresponding parts.

### Tox Core

[jenkins.libtoxcore.so](http://jenkins.libtoxcore.so/job/libtoxcore-win32-i686/lastSuccessfulBuild/artifact/libtoxcore-win32-i686.zip) 
provides a prebuild package of Tox Core. Download this package and extract its content to "C:\qTox\libs". You may have to create the directory "libs".
If you prefer to compile Tox Core on your own follow the instructions at https://github.com/irungentoo/toxcore/blob/master/INSTALL.md#windows

### OpenCV

Unfortunately there are no prebuild packages for OpenCV compiled with MinGW. Thus you have to create your own.
First of all download and install the most recent version of CMake from [cmake.org](http://www.cmake.org/cmake/resources/software.html).
Afterwards download the source archive of OpenCV from [sourceforge.net](http://sourceforge.net/projects/opencvlibrary/) and extract its content to "C:\qTox\libs".
Create a new directory named "opencv-build" in "C:\qTox\libs". Now you should have the two directories "opencv-x.y.z" where x.y.z is the version of OpenCV and "opencv-build" inside your "C:\qTox\libs" directory.
Run CMake Gui and TODO CHECK FOR MSYS MAKE FILES.

### OpenAL Soft

Unlike OpenCV, prebuild packages of OpenAL Soft compiled with MinGW are provided at [http://kcat.strangesoft.net](http://kcat.strangesoft.net/openal.html#download).
Download the most recent version and extract its content to "C:\qTox\libs". Copy the directory "AL" located at "C:\qTox\libs\openal-soft-x.y.z-bin" to "C:\qTox\libs\include" where x.y.z is the version of OpenAL.
Copy the file "soft_oal.dll" located at "C:\qTox\libs\openal-soft-x.y.z-bin\bin\Win32\soft_oal.dll" to "C:\qTox\libs\lib".

## Linux

## Git

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

### GCC and Qt

Debian:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default
```

Ubuntu:
```bash
sudo apt-get install build-essential qt5-qmake qt5-default
```

Arch Linux:
```bash
sudo pacman -S base-devel qt5
```

Fedora:
```bash
yum groupinstall "Development Tools"
yum install qt-devel qt-doc qt-creator
```

### Cloning the Repository

Open a new Terminal and change to a directory of your choice. To clone the repository use:
```bash
git clone https://github.com/tux3/qTox.git qTox
```

The following steps assumes that you cloned the repository at "/home/user/qTox". If you decided to choose another location, replace corresponding parts.

### Tox Core

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
The script will automatically download and install Tox Core and libsodium to "/home/user/qTox/libs"
