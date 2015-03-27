#!/usr/bin/env bash

# Config (Update me if needed !)
VERSION_UPSTREAM="1.0"
VERSION_PACKAGE="1"
PACKAGENAME="qtox"
UPSTREAM_URL="https://github.com/tux3/qTox/archive/master.tar.gz"

# Make some vars for convenience
VERNAME=$PACKAGENAME"_"$VERSION_UPSTREAM
FULLVERNAME=$VERNAME"-"$VERSION_PACKAGE
ARCHIVENAME=$VERNAME".orig.tar.gz"

# ARCHIVENAME > FULLVERNAME > VERNAME = PACKAGENAME+UPVER

# Get some args
OPT_SUDO=true
OPT_APT=true
OPT_KEEP=false
while [ $# -ge 1 ] ; do
    if [ ${1} = "-s" -o ${1} = "--no-sudo" ] ; then
        OPT_SUDO=false
        shift
    elif [ ${1} = "-a" -o ${1} = "--no-apt" ] ; then
        OPT_APT=false
        shift
    elif [ ${1} = "-k" -o ${1} = "--keep" ]; then
        OPT_KEEP=true
        shift
    else
        if [ ${1} != "-h" -a ${1} != "--help" ] ; then
            echo "[ERROR] Unknown parameter \"${1}\""
            echo ""
        fi
    
		# print help
        echo "Use this script to build qTox packages for Debian and Red Hat families"
        echo ""
        echo "usage:"
        echo "    ${0} [-h|--help|-k|--keep|-s|--no-sudo|-a|--no-apt]"
        echo ""
        echo "parameters:"
        echo "    -h|--help   : displays this help"
        echo "    -s|--no-sudo: disables using sudo for apt and alien"
        echo "    -a|--no-apt : disables apt-get (used for build deps) entirely"
        echo "    -k|--keep   : does not delete the build files afterwards"
        echo ""
        echo "example usages:"
        echo "    ${0}       -- build packages, cleaning up trash and running sudo alien and apt-get"
        echo "    ${0} -s -k -- build packages, keeping build files and non-sudo alien and apt-get"
        exit 1
	fi
done

# Get the requried tools if needed
if [[ $OPT_APT = "true" ]]; then
    echo "Installing missing tools (if any)..."
    if [[ $EUID -ne 0 && $OPT_SUDO = "true" ]]; then
        sudo apt-get install wget debhelper cdbs devscripts alien tar gzip build-essential sudo autoconf libtool pkg-config libvpx-dev -y
    else
             apt-get install wget debhelper cdbs devscripts alien tar gzip build-essential sudo autoconf libtool pkg-config libvpx-dev -y
    fi
fi

# Get the requried dependencies if needed
if [[ $OPT_APT = "true" ]]; then
    echo "Installing missing dependencies (if any)..."
    if [[ $EUID -ne 0 && $OPT_SUDO = "true" ]]; then
        sudo apt-get install qt5-qmake libopenal-dev libopencv-dev libopus-dev -y
    else
             apt-get install qt5-qmake libopenal-dev libopencv-dev libopus-dev -y
    fi
fi

mkdir -p .packages
cd .packages

# Cleanup
rm -r $VERNAME 2> /dev/null
rm $ARCHIVENAME 2> /dev/null

# Fectch sources and layout directories
wget -O $ARCHIVENAME $UPSTREAM_URL
tar xvf $ARCHIVENAME 2> /dev/null # Extracts to qTox-master
mv qTox-master $VERNAME
#tar cz $VERNAME > $ARCHIVENAME

# Build packages
cd $VERNAME
./bootstrap.sh -t
debuild -us -uc -aamd64
debuild -us -uc -ai386
cd ..

# alien warns that it should probably be run as root...
if [[ $EUID -ne 0 && $OPT_SUDO = "true" ]]; then
    sudo alien  ./$FULLVERNAME*.deb -r
else
         alien  ./$FULLVERNAME*.deb -r
fi

mv *.deb ..
mv -f *.rpm ..

if [[ $OPT_KEEP = "false" ]]; then
    rm -r *
fi

cd ..
rmdir .packages 2> /dev/null # fails if non empty
