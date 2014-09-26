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
Although the installer provides its own bundled MinGW compiler toolchain its recommend installing it separately because Qt is missing MSYS which is needed to compile and install OpenCV and OpenAL. Thus you can - if needed - deselect the tab "Tools".
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
Afterwards download OpenCV in version 2.4.9 from [sourceforge.net](http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.9/opencv-2.4.9.zip/download) and extract the content of the source archive to "C:\qTox\libs". Furthermore, create a new directory named "opencv-build" in "C:\qTox\libs".
Now you should have the two directories "opencv-2.4.9" and "opencv-build" inside your "C:\qTox\libs" directory.

Run CMake (cmake-gui) and set up the input boxes "Where is the source code:" and "Where to build the binaries" with "C:\qTox\libs\opencv-2.4.9" and "C:\qTox\libs\opencv-build". Press configure and choose "MSYS Makefiles" in the drop down menu with "Use default native compilers". To start initial configuration press Finish. Given that qTox only needs some components of OpenCV it's recommended to disable not required modules. Furthermore, this will decrease compilation time of OpenCV dramatically. Each module begins with "BUILD_opencv_" and can be disabled by deselecting its entry. Use the "Search" input box for convenience. Disable all modules except of "core", "highgui" and "imgproc" (highgui depends on imgproc and will automatically be disabled if imgproc is disabled). For maximum performance search for "CMAKE_BUILD_TYPE" and set this value to "Release". Finally, make sure "CMAKE_INSTALL_PREFIX" points to "C:\qTox\libs\opencv-build\install" (should be by default). To update the configuration press Configure again. To generate the Makefiles press Generate.

Open a new command prompt within "C:\qTox\libs\opencv-build" (HINT: Use shift + right click -> "Open command window here" on the directory within Windows Explorer). Compile and install OpenCV with the following command. It's not recommended to use -j for multicore compilation, because it freezes the terminal from time to time.
```bash
make
make install
```

After OpenCV was successfully installed to "C:\qTox\libs\opencv-build\install" copy the dlls "libopencv_core249.dll", "libopencv_highgui249.dll" and "libopencv_imgproc249.dll" located at "C:\qTox\libs\opencv-build\install\x86\mingw\bin" to "C:\qTox\libs\lib". Afterwards copy the content of the directory "C:\qTox\libs\opencv-build\install\include" to "C:\qTox\libs\include". Finally, you have to patch the file "C:\qTox\libs\include\opencv2\opencv.hpp" because it includes all modules of OpenCV regardless of your configuration. Open this file with your preferred text editor and remove all includes except of "opencv2/core/core_c.h", "opencv2/core/core.hpp", "opencv2/imgproc/imgproc_c.h", "opencv2/imgproc/imgproc.hpp", "opencv2/highgui/highgui_c.h" and "opencv2/highgui/highgui.hpp". OpenCV is now ready to use. Feel free to delete the directories "opencv-2.4.9" and "opencv-build", but you don't need to.

###OpenAL Soft
As for OpenCV there are no prebuild packages of OpenAL Softe compiled with MinGW, but the installation process is very similar to OpenCV. Download the most recent source archive of OpenAL Soft from [http://kcat.strangesoft.net](http://kcat.strangesoft.net/openal.html#download). Extract its content to "C:\qTox\libs". Besides the source folder itself you'll find the file "pax_global_header". It is not required and can be deleted. Create the directory "openal-build" next to source folder. Now you should have the two directories "openal-soft-x.y.z" where x.y.z is the version of OpenAL and "openal-build" inside your "C:\qTox\libs" directory. Run CMake (cmake-gui) and setup the source and build location. Run the initial configuration and use "MSYS Makefiles" with "Use default native compilers". The only thing you need to configure is "CMAKE_INSTALL_PREFIX" which does not point to "C:\qTox\libs\openal-build\install" by default. Configure the project and generate the Makefiles. Compile and install OpenAL Soft with:
```bash
make
make install
```
Copy the dll "OpenAL32.dll" located at "C:\qTox\libs\openal-build\install\bin" to "C:\qTox\libs\lib". Finally, copy the directory "AL" located at "C:\qTox\libs\openal-build\install\include" to "C:\qTox\libs\include". Unlike OpenCV you don't need to patch any files. Feel free to delete the directories "openal-soft-x.y.z" and "openal-build", but you don't need to.

##Linux
Most of the dependencies should be available through your package manger. You may either follow the directions below, or simply run `./simple_make.sh` which will attempt to automatically download dependencies.

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
sudo pacman -S --needed git
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
sudo pacman -S --needed base-devel qt5 opencv openal
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
sudo apt-get install libtool autotools-dev automake checkinstall check libopus-dev libvpx-dev
```

Ubuntu:
```bash
sudo apt-get install libtool autotools-dev automake checkinstall check libopus-dev libvpx-dev
```

Arch Linux: (Arch Linux provides the package "tox-git" in AUR)
```bash
sudo pacman -S --needed opus vpx
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

##Building packages

qTox now has the experimental and probably-dodgy ability to package itself (in .deb
form natively, and .rpm form with <a href="http://joeyh.name/code/alien/">alien</a>).

After installing the required dependencies, run `bootstrap.sh` and then run the
`buildPackages.sh` script, found in the tools folder. It will automatically get the
packages necessary for building .debs, so be prepared to type your password for sudo.

##OS X

###OSX Easy Install

Since https://github.com/ReDetection/homebrew-qtox you can easily install qtox with homebrew 
```bash
brew install --HEAD ReDetection/qtox/qtox
```

###OSX Full Install Guide

This guide is intended for people who wish to use an existing or new ProjectTox-Core installation separate to the bundled installation with qTox, if you do not wish to use a separate installation you can skip to the section titled 'Final Steps'.

Installation on OSX, isn't quite straight forward, here is a quick guide on how to install;

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
```
Assuming all went well you should now have a qTox.app file within the directory. Double click and it should open!
