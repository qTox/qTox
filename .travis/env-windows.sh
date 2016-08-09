#!/bin/sh

RUN() {
  ./dockcross "$@"
}
export CMAKE=$ARCH-w64-mingw32.$LIBTYPE-cmake
export MAKE=make
export NPROC=`nproc`
export CURDIR=/work
