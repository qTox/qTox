#!/bin/bash

# Config (Update me if needed !)
VERSION_UPSTREAM="1.0"
VERSION_PACKAGE="1"
PACKAGENAME="qtox"
UPSTREAM_GIT_URL="https://github.com/tux3/qtox.git"

# Make some vars for convenience
VERNAME=$PACKAGENAME"_"$VERSION_UPSTREAM
FULLVERNAME=$VERNAME"-"$VERSION_PACKAGE
ARCHIVENAME=$VERNAME".orig.tar.gz"

# Get the requried tools if needed
echo "Installing missing tools (if any)..."
apt-get install git debhelper cdbs devscripts alien tar gzip build-essential

# Cleanup
rm -r $VERNAME 2> /dev/null
rm $ARCHIVENAME 2> /dev/null

# Fectche sources and layour directories
git clone --depth 1 $UPSTREAM_GIT_URL
mv $PACKAGENAME $VERNAME
tar cz $VERNAME > $ARCHIVENAME

# Build packages
cd $VERNAME
debuild -us -uc
cd ..
alien  ./$FULLVERNAME*.deb -r
