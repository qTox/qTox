#!/usr/bin/env bash

WINDOWS_VERSION=$(cmd.exe /c ver 2>/dev/null | grep "Microsoft Windows")
if [ ! -z "$WINDOWS_VERSION" ]; then
        cd windows
		./bootstrap.sh
		exit $?
fi

################ parameters ################
# directory where the script is located
SCRIPT_DIR=$( cd $(dirname $0); pwd -P)

# directory where dependencies will be installed
INSTALL_DIR=libs

# just for convenience
BASE_DIR=${SCRIPT_DIR}/${INSTALL_DIR}

# the sodium version to use
SODIUM_VER=1.0.2

# directory names of cloned repositories
SODIUM_DIR=libsodium-$SODIUM_VER
TOX_CORE_DIR=libtoxcore-latest
FILTER_AUDIO_DIR=libfilteraudio-latest

if [ -z "$BASE_DIR" ]; then
    echo "internal error detected!"
    echo "BASE_DIR should not be empty... aborting"
    exit 1
fi

if [ -z "$SODIUM_DIR" ]; then
    echo "internal error detected!"
    echo "SODIUM_DIR should not be empty... aborting"
    exit 1
fi

if [ -z "$TOX_CORE_DIR" ]; then
    echo "internal error detected!"
    echo "TOX_CORE_DIR should not be empty... aborting"
    exit 1
fi

if [ -z "$FILTER_AUDIO_DIR" ]; then
    echo "internal error detected!"
    echo "FILTER_AUDIO_DIR should not be empty... aborting"
    exit 1
fi

# default values for user given parameters
INSTALL_SODIUM=true
INSTALL_TOX=true
INSTALL_FILTER_AUDIO=true
SYSTEM_WIDE=true
KEEP_BUILD_FILES=false


########## parse input parameters ##########
while [ $# -ge 1 ] ; do
    if [ ${1} = "--with-sodium" ] ; then
        INSTALL_SODIUM=true
        shift
    elif [ ${1} = "--without-sodium" ] ; then
        INSTALL_SODIUM=false
        shift
    elif [ ${1} = "--with-tox" ] ; then
        INSTALL_TOX=true
        shift
    elif [ ${1} = "--without-tox" ] ; then
        INSTALL_TOX=false
        shift
    elif [ ${1} = "--with-filter-audio" ] ; then
        INSTALL_FILTER_AUDIO=true
        shift
    elif [ ${1} = "--without-filter-audio" ] ; then
        INSTALL_FILTER_AUDIO=false
        shift
    elif [ ${1} = "-l" -o ${1} = "--local" ] ; then
        SYSTEM_WIDE=false
        shift
    elif [ ${1} = "-k" -o ${1} = "--keep" ]; then
        KEEP_BUILD_FILES=true
        shift
    else
        if [ ${1} != "-h" -a ${1} != "--help" ] ; then
            echo "[ERROR] Unknown parameter \"${1}\""
            echo ""
        fi
    
        # print help
        echo "Use this script to install/update libsodium, libtoxcore and libfilteraudio"
        echo ""
        echo "usage:"
        echo "    ${0} PARAMETERS"
        echo ""
        echo "parameters:"
        echo "    --with-sodium          : install/update libsodium"
        echo "    --without-sodium       : do not install/update libsodium"
        echo "    --with-tox             : install/update libtoxcore"
        echo "    --without-tox          : do not install/update libtoxcore"
        echo "    --with-filter-audio    : install/update libfilteraudio"
        echo "    --without-filter-audio : do not install/update libfilteraudio"
        echo "    -h|--help              : displays this help"
        echo "    -l|--local             : install packages into ${INSTALL_DIR}"
        echo "    -k|--keep              : keep build files after installation/update"
        echo ""
        echo "example usages:"
        echo "    ${0}    -- install libsodium, libtoxcore and libfilteraudio"
        exit 1
	fi
done


########## print debug output ##########
echo "with sodium                 : ${INSTALL_SODIUM}"
echo "with tox                    : ${INSTALL_TOX}"
echo "with filter-audio           : ${INSTALL_FILTER_AUDIO}"
echo "install into ${INSTALL_DIR} : ${SYSTEM_WIDE}"
echo "keep build files            : ${KEEP_BUILD_FILES}"


############### prepare step ###############
# create BASE_DIR directory if necessary
mkdir -p ${BASE_DIR}

# maybe an earlier run of this script failed
# thus we should remove the cloned repositories
# if exists, otherwise cloning them may fail
rm -rf ${BASE_DIR}/${SODIUM_DIR}
rm -rf ${BASE_DIR}/${TOX_CORE_DIR}
rm -rf ${BASE_DIR}/${FILTER_AUDIO_DIR}


############### install step ###############
# install libsodium
if [[ $INSTALL_SODIUM = "true" ]]; then
	git clone --branch $SODIUM_VER git://github.com/jedisct1/libsodium.git ${BASE_DIR}/${SODIUM_DIR} --depth 1
	pushd ${BASE_DIR}/${SODIUM_DIR}
	./autogen.sh
	
	if [[ $SYSTEM_WIDE = "false" ]]; then
        ./configure --prefix=${BASE_DIR}
        make -j2 check
        make install
    else
        ./configure
        make -j2 check
        sudo make install
        sudo ldconfig
    fi
    
    popd
fi

#install libtoxcore
if [[ $INSTALL_TOX = "true" ]]; then
	git clone https://github.com/irungentoo/toxcore.git ${BASE_DIR}/${TOX_CORE_DIR} --depth 1
	pushd ${BASE_DIR}/${TOX_CORE_DIR}
	./autogen.sh
	
	if [[ $SYSTEM_WIDE = "false" ]]; then
        ./configure --prefix=${BASE_DIR} --with-libsodium-headers=${BASE_DIR}/include --with-libsodium-libs=${BASE_DIR}/lib
        make -j2
        make install
    else
        ./configure
        make -j2
        sudo make install
        sudo ldconfig
    fi
    
    popd
fi

#install libfilteraudio
if [[ $INSTALL_FILTER_AUDIO = "true" ]]; then
	git clone https://github.com/irungentoo/filter_audio.git ${BASE_DIR}/${FILTER_AUDIO_DIR} --depth 1
	pushd ${BASE_DIR}/${FILTER_AUDIO_DIR}
	
	if [[ $SYSTEM_WIDE = "false" ]]; then
		PREFIX=${BASE_DIR} make -j2
		PREFIX=${BASE_DIR} make install
	else
		make -j2
		sudo make install
		sudo ldconfig
	fi
	
	popd
fi


############### cleanup step ###############
# remove cloned repositories
if [[ $KEEP_BUILD_FILES = "false" ]]; then
    rm -rf ${BASE_DIR}/${SODIUM_DIR}
    rm -rf ${BASE_DIR}/${TOX_CORE_DIR}
    rm -rf ${BASE_DIR}/${FILTER_AUDIO_DIR}
fi
