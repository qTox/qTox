#!/bin/sh

RUN() {
  "$@"
}
export CMAKE=cmake
export MAKE=make
export NPROC=`sysctl -n hw.ncpu`
export CURDIR=$PWD

export CMAKE_PREFIX_PATH=`brew --prefix qt5`
