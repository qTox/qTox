#!/bin/bash

################ parameters ################
# directory where the script is located
SCRIPT_NAME=$(readlink -f $0)
SCRIPT_DIR=`dirname $SCRIPT_NAME`

# directory where dependencies will be installed
INSTALL_DIR=libs

# just for convenience
BASE_DIR=${SCRIPT_DIR}/${INSTALL_DIR}

# directory names of cloned repositories
SODIUM_DIR=libsodium-0.5.0
TOX_CORE_DIR=libtoxcore-latest

# this boolean describes whether the installation of
# libsodium should be skipped or not
# the default value is 'false' and will be set to 'true'
# if this script gets the parameter -t or --tox
TOX_ONLY=false



########## check input parameters ##########
if [ $# -ge 1 ] ; then
    if [ ${1} = "-t" -o ${1} = "--tox" ] ; then
        TOX_ONLY=true
    else
        if [ ${1} != "-h" -a ${1} != "--help" ] ; then
            echo "[ERROR] Unknown parameter \"${1}\""
            echo ""
        fi
    
		# print help
		echo "Use this script to install/update libsodium and libtoxcore in ${INSTALL_DIR}"
		echo ""
		echo "usage:"
		echo "    ${0} [-t|--tox|-h|--help]"
		echo ""
		echo "parameters:"
		echo "    -h|--help: displays this help"
		echo "    -t|--tox : only install/update libtoxcore"
		echo "               requires an already installed libsodium"
		echo ""
		echo "example usages:"
		echo "    ${0}    -- to install libsodium and libtoxcore"
		echo "    ${0} -t -- to update already installed libtoxcore"
		exit 1
	fi
fi



############### prepare step ###############
# create INSTALL_DIR directory if necessary
mkdir -p ${BASE_DIR}

# maybe an earlier run of this script failed
# thus we should remove the cloned repositories
# if exists, otherwise cloning them may fail
rm -rf ${BASE_DIR}/${SODIUM_DIR}
rm -rf ${SBASE_DIR}/${TOX_CORE}



############### install step ###############
# clone current master of libsodium and switch to version 0.5.0
# afterwards install libsodium to INSTALL_DIR
# skip the installation if TOX_ONLY is true
if [[ $TOX_ONLY = "false" ]]; then
    git clone git://github.com/jedisct1/libsodium.git ${BASE_DIR}/${SODIUM_DIR}
    pushd ${BASE_DIR}/${SODIUM_DIR}
    git checkout tags/0.5.0
    ./autogen.sh
    ./configure --prefix=${BASE_DIR}/
    make -j2 check
    make install
    popd
fi

# clone current master of libtoxcore
# make sure to compile with libsodium we just installed to INSTALL_DIR
# afterwards install libtoxcore to INSTALL_DIR
git clone https://github.com/irungentoo/toxcore.git ${BASE_DIR}/${TOX_CORE_DIR}
pushd ${BASE_DIR}/${TOX_CORE_DIR}
./autogen.sh
./configure --prefix=${BASE_DIR}/ --with-libsodium-headers=${BASE_DIR}/include --with-libsodium-libs=${BASE_DIR}/lib
make -j2
make install
popd



############### cleanup step ###############
# remove cloned repositories
rm -rf ${BASE_DIR}/${SODIUM_DIR}
rm -rf ${BASE_DIR}/${TOX_CORE_DIR}
