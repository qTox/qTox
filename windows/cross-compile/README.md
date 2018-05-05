# Cross-compile from Linux to Windows

## Intro

Following these instructions will show you how to cross-compile qTox for Windows.

This script can be used by qTox users and devs to compile qTox for windows themselves.

Please note that the compilation script doesn't build the updater and doesn't produce
an installer.

## Usage


#### 1. [Install Docker](https://docs.docker.com/install).
***


#### 2. Create 3 directories for storing the files needed for cross-compiling.
***

  * `workspace` -- a directory that will contain a cache of qTox dependencies
  and the final qTox cross-compilation build. You should create this directory.

  * `script` -- a directory that contains the `build.sh` script. 
  Copy the `build.sh` script to this folder.

  * `qtox` -- the root directory of a qTox repository. This directory must contain
  the qTox source code that will be cross-compiled. You can use an existing qTox
  directory you've been using for development or check one out using 
  `git clone https://github.com/qTox/qTox.git folder-name`

These directories will be mounted inside a Docker container at`/workspace`,
`/script`and`/qtox`.

The contents of `qtox` and `script` directories are not modified during compilation. The
`build.sh` script makes a temporary copy of the `qtox` directory for compilation.


#### 3. Create the docker container and run the build script.
***
> Note that the`build.sh`script takes 2 arguments: architecture and build type.
> Valid values for the architecture are `i686` for 32-bit and `x86_64` for
> 64-bit. Valid values for the build type are `release` and `debug`. All case
> sensitive. You can modify the scripts below to fit your use case.
> To create the docker container and start cross-compiling run



```sh
sudo docker run --rm \
-v /absolute/path/to/your/workspace:/workspace \
-v /absolute/path/to/your/script:/script \
-v /absolute/path/to/your/qtox:/qtox \
debian:stretch-slim \
/bin/bash /script/build.sh i686 release
```


If you are a qTox developer, you might want to enable tty and leave stdin open by running the following script instead.

```sh
sudo docker run -it \
--rm \
-v /absolute/path/to/your/workspace:/workspace \
-v /absolute/path/to/your/script:/script \
-v /absolute/path/to/your/qtox:/qtox \
debian:stretch-slim \
/bin/bash /script/build.sh i686 release
```
These will cross-compile all of the qTox dependencies and qTox itself, storing
them in the `workspace` directory. The first time you run it for each
architecture, it will take a long time for the cross-compilation to finish, as
qTox has a lot of dependencies that need to be cross-compiled.


> Note that it takes my Intel Core i7 processor about 125 minutes on average for the cross-compilation
> to finish on a single core, and about 30 minutes using all 8 hyperthreads. Once you've compiled
> it, the dependencies will be cached inside the `workspace` directory. The next time
> you build qTox, the `build.sh` script will skip recompiling them
> which is a lot faster -- about 8 minutes on a single core and 2 minutes using 8 hyperthreads.


#### 4. After cross-compiling has finished
***
The `workspace\i686\qtox` and `workspace\x86_64\qtox\`directories will contain the compiled binaries in their respective debug or release folder depending on the compilation settings you chose in Step 3.

The `dep-cache` directory is where all the cross-compiled qTox dependencies will be
cached for the future builds. You can remove any directory inside the `deps` folder, which
will result in the `build.sh` re-compiling the removed dependency.


_The `workspace` direcory structure for reference_

```
workspace
├── i686
│   ├── dep-cache
│   │   ├── libffmpeg
│   │   ├── libfilteraudio
│   │   ├── libopenal
│   │   ├── libopenssl
│   │   ├── libopus
│   │   ├── libqrencode
│   │   ├── libqt5
│   │   ├── libsodium
│   │   ├── libsqlcipher
│   │   ├── libtoxcore
│   │   └── libvpx
│   └── qtox
│       ├── debug
│       └── release
└── x86_64
    ├── dep-cache
    │   ├── libffmpeg
    │   ├── libfilteraudio
    │   ├── libopenal
    │   ├── libopenssl
    │   ├── libopus
    │   ├── libqrencode
    │   ├── libqt5
    │   ├── libsodium
    │   ├── libsqlcipher
    │   ├── libtoxcore
    │   └── libvpx
    └── qtox
        ├── debug
        └── release
```
