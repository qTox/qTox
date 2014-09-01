#!/bin/bash

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

# Get the requried tools if needed
echo "Installing missing tools (if any)..."
if [[ $EUID -ne 0 ]]; then
  sudo apt-get install wget debhelper cdbs devscripts alien tar gzip build-essential
else
  apt-get install wget debhelper cdbs devscripts alien tar gzip build-essential
fi

mkdir -p .packages
cd .packages

# Cleanup
rm -r $VERNAME 2> /dev/null
rm $ARCHIVENAME 2> /dev/null

# Fectch sources and layout directories
wget -O $ARCHIVENAME $UPSTREAM_URL
tar xvf $ARCHIVENAME # Extracts to qTox-master
mv qTox-master $VERNAME
#tar cz $VERNAME > $ARCHIVENAME

# Build packages
cd $VERNAME
debuild -us -uc
cd ..

# alien warns that it should probably be run as root...
if [[ $EUID -ne 0 ]]; then
  sudo alien  ./$FULLVERNAME*.deb -r
else
  alien  ./$FULLVERNAME*.deb -r
fi

mv *.deb ..
mv *.rpm ..

rm -r *
cd ..
rmdir .packages
