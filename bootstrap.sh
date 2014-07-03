#!/bin/bash

# directory where the script is located
SCRIPT_NAME=$(readlink -f $0)
SCRIPT_DIR=`dirname $SCRIPT_NAME`

# create libs dir if necessary
mkdir -p ${SCRIPT_DIR}/libs

# clone current master of libtoxcore
git clone https://github.com/irungentoo/toxcore.git ${SCRIPT_DIR}/libs/libtoxcore-latest

# compile and install libtoxcore
pushd ${SCRIPT_DIR}/libs/libtoxcore-latest
./autogen.sh
./configure --prefix=${SCRIPT_DIR}/libs/
make -j2
make install
popd

# remove clone dir
rm -rf ${SCRIPT_DIR}/libs/libtoxcore-latest


