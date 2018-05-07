# Cross-compile from Linux to Windows

## Intro

Following these instructions you will be able to cross-compile qTox for
Windows.

This script can be used by qTox users and devs to compile qTox for Windows
themselves.

Please note that the compilation script doesn't build the updater.

## Usage

[Install Docker](https://docs.docker.com/install).

Create 3 directories:

  * `workspace` -- a directory that will contain a cache of qTox dependencies
  and the final qTox cross-compilation build. You should create this directory.

  * `qtox` -- the root directory of a qTox repository. This directory must
  contain the qTox source code that will be cross-compiled.

  * `script` -- a directory that contains the `build.sh` script. You could
  create this directory and copy the `build.sh` script in it, or you
  could just use the `qtox/windows/cross-compile` directory for this.

These directories will be mounted inside a Docker container at `/workspace`,
`/qtox` and `/script`.

The contents of `qtox` and `script` directories are not modified during
compilation. The `build.sh` script makes a temporary copy of the `qtox`
directory for compilation.

Once you sort out the directories, you are ready to run the `build.sh` script
in a Docker container.

> Note:
>     The`build.sh` script takes 2 arguments: architecture and build type.
>     Valid values for the architecture are `i686` for 32-bit and `x86_64` for
>     64-bit. Valid values for the build type are `release` and `debug`. All
>     case sensitive.

To start cross-compiling for 32-bit release version of qTox run:


```sh
sudo docker run --rm \
                -v /absolute/path/to/your/workspace:/workspace \
                -v /absolute/path/to/your/script:/script \
                -v /absolute/path/to/your/qtox:/qtox \
                debian:stretch-slim \
                /bin/bash /script/build.sh i686 release
```

If you want to debug some compilation issue, you might want to instead run:

```sh
# Get shell inside Debian Stretch container so that you can poke around if needed
sudo docker run -it \
                --rm \
                -v /absolute/path/to/your/workspace:/workspace \
                -v /absolute/path/to/your/script:/script \
                -v /absolute/path/to/your/qtox:/qtox \
                debian:stretch-slim \
                /bin/bash
# Run the script
bash /script/build.sh i686 release
```

These will cross-compile all of the qTox dependencies and qTox itself, storing
them in the `workspace` directory. The first time you run it for each
architecture, it will take a long time for the cross-compilation to finish, as
qTox has a lot of dependencies that need to be cross-compiled. But once you do
it once for each architecture, the dependencies will get cached inside the
`workspace` directory, and the next time you build qTox, the `build.sh` script
will skip recompiling them, going straight to compiling qTox, which is a lot
faster.

> Note:
>     On a certain Intel Core i7 processor, a fresh build takes about 125
>     minutes on a single core, and about 30 minutes using all 8 hyperthreads.
>     Once built, however, it takes about 8 minutes on a single core and 2
>     minutes using 8 hyperthreads to rebuild using the cached dependencies.

After cross-compiling has finished, you should find the comiled qTox in a
`workspace\i686\qtox` or `workspace\x86_64\qtox\` directory, depending on the
architecture.

You will also find `workspace\dep-cache` directory, where all the
cross-compiled qTox dependencies will be cached for the future builds. You can
remove any directory inside the `dep-cache`, which will result in the
`build.sh` re-compiling the removed dependency only.


_The `workspace` direcory structure for reference_

```
workspace/
├── i686
│   ├── dep-cache
│   │   ├── libexif
│   │   ├── libffmpeg
│   │   ├── libfilteraudio
│   │   ├── libopenal
│   │   ├── libopenssl
│   │   ├── libopus
│   │   ├── libqrencode
│   │   ├── libqt5
│   │   ├── libsodium
│   │   ├── libsqlcipher
│   │   ├── libtoxcore
│   │   ├── libvpx
│   │   ├── mingw-w64-debug-scripts
│   │   ├── nsis
│   │   └── nsis_shellexecuteasuser
│   └── qtox
│       ├── debug
│       └── release
└── x86_64
    ├── dep-cache
    │   ├── libexif
    │   ├── libffmpeg
    │   ├── libfilteraudio
    │   ├── libopenal
    │   ├── libopenssl
    │   ├── libopus
    │   ├── libqrencode
    │   ├── libqt5
    │   ├── libsodium
    │   ├── libsqlcipher
    │   ├── libtoxcore
    │   ├── libvpx
    │   ├── mingw-w64-debug-scripts
    │   ├── nsis
    │   └── nsis_shellexecuteasuser
    └── qtox
        ├── debug
        └── release
```
