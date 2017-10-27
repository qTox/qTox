# Cross-compile from Linux

## Intro

Following these instructions you will be able to cross-compile qTox to Windows.

This script can be used by qTox power users to cross-compile qTox themselves,as
 well as by qTox developers to make sure their local changes don't break
cross-compilation to Windows.

Note that the compilation script doesn't build the updater and doesn't produce
an installer.

## Usage

[Install Docker](https://docs.docker.com/engine/installation/linux/).

You should have 3 directories available (names don't have to match the given
ones):

1. `workspace` -- a directory that will contain a cache of qTox dependencies
and the final qTox cross-compilation build. You should create this directory.
2. `script` -- a directory that contains the `build.sh` script. You can use
this directory for this, there is no need to create a new one.
3. `qtox` -- a root directory of a qTox repository. This directory contains
qTox source code that will be cross-compiled. You can use the root of this qTox
repository, there is no need to create a new one.

These directories will be mounted inside a Docker container at `/workspace`,
`/script` and `/qtox`.

The contents of `qtox` and `script` directories wouldn't be modified. The
`build.sh` script actually makes a copy of `qtox` directory and works on that
copy.

Once you sort out the directories, you are ready to run the `build.sh` script
in a Docker container.

Note that the `build.sh` script takes 2 arguments: architecture and build type.
Valid values for the architecture are `i686` for 32-bit and `x86_64` for
64-bit. Valid values for the build type are `release` and `debug`. All case
sensitive.

Now, to start the cross-compilation, for example, for a 32-bit release qTox, run

```sh
sudo docker run --rm \
                -v /absolute/path/to/your/workspace:/workspace \
                -v /absolute/path/to/your/script:/script \
                -v /absolute/path/to/your/qtox:/qtox \
                debian:stretch-slim \
                /bin/bash /script/build.sh i686 release
```

If you are a qTox developer, you might want to instead run

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

This will cross-compile all of the qTox dependencies and qTox itself, storing
them in the `workspace` directory. The first time you run it for each
architecture, it will take a long time for the cross-compilation to finish, as
qTox has a lot of dependencies that need to be cross-compiled. It takes my
Intel Core i7 processor about 125 minutes for the cross-compilation to get done
on a single core, and about 30 minutes using all 8 hyperthreads. But once you
do it once for each architecture, the dependencies will get cached inside the
`workspace` directory, and the next time you build qTox, the `build.sh` script
will skip recompiling them, going straight to compiling qTox, which is a lot
faster -- about 8 minutes on a single core and 2 minutes using 8 hyperthreads.

The structure of `workspace` directory that the `build.sh` script will create
is as follows:

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

The `dep-cache` directory is where all cross-compiled qTox dependencies will be
cached for the future runs. You can remove any directory inside `deps`, which
will result in the `build.sh` re-compiling the missing dependency.

The `qtox` directory, split into `debug` and `release`, is where the
cross-compiled qTox will be stored.
