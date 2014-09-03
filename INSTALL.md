###Dependencies

| Name         | Version     | Modules                         |
|--------------|-------------|-------------------------------- |
| Qt           | >= 5.2.0    | core, gui, network, widget, xml |
| GCC/MinGW    | >= 4.6      | C++11 enabled                   |
| Tox Core     | most recent | core, av                        |
| OpenCV       | >= 2.4.9    | core, highgui                   |
| OpenAL Soft  | >= 1.16.0   |                                 |

##Windows

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
provides a prebuild package of Tox Core. Download this package and extract its content to "C:\qTox\libs". You may have to create the folder "libs".

