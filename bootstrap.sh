#!/usr/bin/env bash

# This script's purpose is to ease compiling qTox for users.
# 
# NO AUTOMATED BUILDS SHOULD DEPEND ON IT.
# 
# This script is and will be a subject to breaking changes, and at no time one
# should expect it to work - it's something that you could try to use but
# don't expect that it will work for sure.
#
# If script doesn't work, you should use instructions provided in INSTALL.md
# before reporting issues like “qTox doesn't compile”.
#
# With that being said, reporting that this script doesn't work would be nice.
#
# If you are contributing code to qTox that change its dependencies / the way
# it's being build, please keep in mind that changing just bootstrap.sh 
# *IS NOT* and will not be sufficient - you should update INSTALL.md first.


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

# directory names of cloned repositories
TOX_CORE_DIR=libtoxcore-latest
FILTER_AUDIO_DIR=libfilteraudio-latest
SNORENOTIFY_DIR=libsnore-latest

if [ -z "$BASE_DIR" ]; then
    echo "internal error detected!"
    echo "BASE_DIR should not be empty.  Aborting."
    exit 1
fi

if [ -z "$TOX_CORE_DIR" ]; then
    echo "internal error detected!"
    echo "TOX_CORE_DIR should not be empty.  Aborting."
    exit 1
fi

if [ -z "$FILTER_AUDIO_DIR" ]; then
    echo "internal error detected!"
    echo "FILTER_AUDIO_DIR should not be empty.  Aborting."
    exit 1
fi

if [ -z "$SNORENOTIFY_DIR" ]; then
    echo "internal error detected!"
    echo "SNORENOTIFY_DIR should not be empty.  Aborting."
    exit 1
fi

# default values for user given parameters
INSTALL_TOX=true
INSTALL_FILTER_AUDIO=true
INSTALL_SNORENOTIFY=true
SYSTEM_WIDE=true
KEEP_BUILD_FILES=false


########## parse input parameters ##########
while [ $# -ge 1 ] ; do
    if [ ${1} = "--with-tox" ] ; then
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
    elif [ ${1} = "--with-snorenotify" ] ; then
        INSTALL_SNORENOTIFY=true
        shift
    elif [ ${1} = "--without-snorenotify" ] ; then
        INSTALL_SNORENOTIFY=false
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
        echo "Use this script to install/update libtoxcore and libfilteraudio"
        echo ""
        echo "usage:"
        echo "    ${0} PARAMETERS"
        echo ""
        echo "parameters:"
        echo "    --with-tox             : install/update libtoxcore"
        echo "    --without-tox          : do not install/update libtoxcore"
        echo "    --with-filter-audio    : install/update libfilteraudio"
        echo "    --without-filter-audio : do not install/update libfilteraudio"
        echo "    --with-snorenotify     : install/update Snorenotify"
        echo "    --without-snorenotify  : do not install/update Snorenotify"
        echo "    -h|--help              : displays this help"
        echo "    -l|--local             : install packages into ${INSTALL_DIR}"
        echo "    -k|--keep              : keep build files after installation/update"
        echo ""
        echo "example usages:"
        echo "    ${0}    -- install libtoxcore and libfilteraudio"
        exit 1
    fi
done


############ print debug output ############
echo "with tox                    : ${INSTALL_TOX}"
echo "with filter-audio           : ${INSTALL_FILTER_AUDIO}"
echo "with Snorenotify            : ${INSTALL_SNORENOTIFY}"
echo "install into ${INSTALL_DIR} : ${SYSTEM_WIDE}"
echo "keep build files            : ${KEEP_BUILD_FILES}"


############### prepare step ###############
# create BASE_DIR directory if necessary
mkdir -p ${BASE_DIR}

# maybe an earlier run of this script failed
# thus we should remove the cloned repositories
# if exists, otherwise cloning them may fail
rm -rf ${BASE_DIR}/${TOX_CORE_DIR}
rm -rf ${BASE_DIR}/${FILTER_AUDIO_DIR}
rm -rf ${BASE_DIR}/${SNORENOTIFY_DIR}


############### install step ###############
#install libtoxcore
if [[ $INSTALL_TOX = "true" ]]; then
    git clone https://github.com/irungentoo/toxcore.git ${BASE_DIR}/${TOX_CORE_DIR} --depth 1
    pushd ${BASE_DIR}/${TOX_CORE_DIR}
    ./autogen.sh
    
    # configure
    if [[ $SYSTEM_WIDE = "false" ]]; then
        ./configure --prefix=${BASE_DIR}
    else
        ./configure
    fi
    
    # ensure A/V support is enabled
    if ! grep -Fxq "BUILD_AV_TRUE=''" config.log
    then
        echo "A/V support of libtoxcore is disabled but required by qTox.  Aborting."
        echo "Maybe the dev-packages of libopus and libvpx are not installed?"
        exit 1
    fi
    
    # compile
    make -j 2
    
    # install
    if [[ $SYSTEM_WIDE = "false" ]]; then
        make install
    else
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

#install Snorenotify
if [[ $INSTALL_SNORENOTIFY = "true" ]]; then
    git clone https://github.com/Snorenotify/Snorenotify.git ${BASE_DIR}/${SNORENOTIFY_DIR} --depth 1
    pushd ${BASE_DIR}/${SNORENOTIFY_DIR}
    
    if [[ $SYSTEM_WIDE = "false" ]]; then
        cmake -qt5 -DCMAKE_INSTALL_PREFIX="${BASE_DIR}" .
        make -j2
        make install
    else
    	cmake -qt5 .
        make -j2
        sudo make install
        sudo ldconfig
    fi
    
    popd
fi

############### cleanup step ###############
# remove cloned repositories
if [[ $KEEP_BUILD_FILES = "false" ]]; then
    rm -rf ${BASE_DIR}/${TOX_CORE_DIR}
    rm -rf ${BASE_DIR}/${FILTER_AUDIO_DIR}
    rm -rf ${BASE_DIR}/${SNORENOTIFY_DIR}
fi
