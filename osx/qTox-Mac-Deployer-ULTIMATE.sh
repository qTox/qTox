#!/usr/bin/env bash

#
#    Copyright © 2015 by RowenStipe
#    Copyright © 2016 by The qTox Project
#
#    This program is libre software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This uses the same process as doing it manually but with a few varients

# Use: ./qTox-Mac-Deployer-ULTIMATE.sh -h

# Your home DIR really (Most of this happens in it) {DONT USE: ~ }
if [[ $TRAVIS = true ]]; then #travis check
	MAIN_DIR="/Users/${USER}/${TRAVIS_BUILD_DIR}"
else
	MAIN_DIR="/Users/${USER}"
fi
QT_DIR="/usr/local/Cellar/qt5" # Folder name of QT install
VER="${QT_DIR}/5.5.1_2" # Potential future proffing for version testing
QMAKE="${VER}/bin/qmake" # Don't change
MACDEPLOYQT="${VER}/bin/macdeployqt" # Don't change

QTOX_DIR="${MAIN_DIR}/qTox" # Change to Git location

TOXCORE_DIR="${MAIN_DIR}/toxcore" # Change to Git location

FA_DIR="${MAIN_DIR}/filter_audio"

BUILD_DIR="${MAIN_DIR}/qTox-Mac_Build" # Change if needed

DEPLOY_DIR="${MAIN_DIR}/qTox-Mac_Deployed"


function fcho() {
	local msg="$1"; shift
	printf "\n$msg\n" "$@"
}

function build-toxcore() {
	echo "Starting Toxcore build and install"
	cd $TOXCORE_DIR
	echo "Now working in: ${PWD}"
	
	#Check if libsodium is correct version
	if [ -e /usr/local/opt/libsodium/lib/libsodium.17.dylib ]; then
	   	fcho " Beginnning Toxcore compile "
  	else
		echo "Error: libsodium.17.dylib not found! Unable to build!"
		echo "Please make sure your Homebrew packages are up to date before retrying."
		exit 1
	fi
	sleep 3
	
	autoreconf -i 
	
	#Make sure the correct version of libsodium is used
	./configure --with-libsodium-headers=/usr/local/Cellar/libsodium/1.0.6/include/ --with-libsodium-libs=/usr/local/Cellar/libsodium/1.0.6/lib/
	
	sudo make clean
	make	
	echo "------------------------------"
	echo "Sudo required, please enter your password:"
	sudo make install
}

function install() {
	fcho "=============================="
	fcho "This script will install the nessicarry applications and libraries needed to compile qTox properly."
	fcho "Note that this is not a 100 percent automated install it just helps simplfiy the process for less experianced or lazy users."
	if [[ $TRAVIS = true ]]; then #travis check
		echo "Oh... It's just Travis...."
	else
	read -n1 -rsp $'Press any key to continue or Ctrl+C to exit...\n'
	fi
		
	if [ -e /usr/local/bin/brew ]; then
		fcho "Homebrew already installed!"
	else
		fcho "Installing homebrew ..."
		ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
	fi
	fcho "Updating brew formulas ..."
	brew update
	fcho "Getting home brew formulas ..."
	sleep 3
	brew install git ffmpeg qrencode wget libtool automake autoconf libsodium check qt5
	
	fcho "Installing x-code Comand line tools ..."
	xcode-select --install
	
	fcho "Starting git repo checks ..."
	
	cd $MAIN_DIR # just in case
	if [ -e $TOX_DIR/.git/index ]; then # Check if this exists
		fcho "Toxcore git repo already inplace !"
		cd $TOX_DIR
		git pull
	else
		fcho "Cloning Toxcore git ... "
		git clone https://github.com/irungentoo/toxcore.git
	fi
	if [ -e $QTOX_DIR/.git/index ]; then # Check if this exists
		fcho "qTox git repo already inplace !"
		cd $QTOX_DIR
		git pull
	else
		fcho "Cloning qTox git ... "
		git clone https://github.com/tux3/qTox.git
	fi
	if [ -e $FA_DIR/.git/index ]; then # Check if this exists
		fcho "Filter_Audio git repo already inplace !"
		cd $FA_DIR
		git pull
		fcho "Please enter your password to install Filter_Audio:"
		sudo make install
	else
		fcho "Cloning Filter_Audio git ... "
		git clone https://github.com/irungentoo/filter_audio.git
		cd $FA_DIR
		fcho "Please enter your password to install Filter_Audio:"
		sudo make install
	fi
	if [[ $TRAVIS = true ]]; then #travis check
		build-toxcore
	else
		fcho "If all went well you should now have all the tools needed to compile qTox!"
		read -r -p "Would you like to install toxcore now? [y/N] " response
		if [[ $response =~ ^([yY][eE][sS]|[yY])$ ]]; then
			build-toxcore	
		else
		    fcho "You can simply use the -u command and say [Yes/n] when prompted"
		fi
	fi
}

function update() {
	fcho "------------------------------"
	fcho "Starting update process ..."	
	#First update Toxcore from git
	cd $TOXCORE_DIR
	fcho "Now in ${PWD}"
	fcho "Pulling ..."
	git pull
	read -r -p "Did Toxcore update from git? [y/N] " response
	if [[ $response =~ ^([yY][eE][sS]|[yY])$ ]]; then
		build-toxcore	
	else
	    fcho "Moving on!"
	fi
	
	#Now let's update qTox!
	cd $QTOX_DIR
	fcho "Now in ${PWD}"
	fcho "Pulling ..."
	git pull
	read -r -p "Did qTox update from git? [y/N] " response
	if [[ $response =~ ^([yY][eE][sS]|[yY])$ ]]; then
		fcho "Starting OSX bootstrap ..."
		fcho "Sudo required:"
		sudo bash ./bootstrap-osx.sh
	else
	    fcho "Moving on!"
	fi
}

function build() {
	fcho "------------------------------"
	fcho "Starting build process ..."
	rm -r $BUILD_DIR
	rm -r $DEPLOY_DIR
	mkdir $BUILD_DIR
	cd $BUILD_DIR
	fcho "Now working in ${PWD}"
	fcho "Starting qmake ... "
	$QMAKE $QTOX_DIR/qtox.pro
	make
}

function deploy() {
	fcho "------------------------------"
	fcho "starting deployment process ..."
	cd $BUILD_DIR
	if [ ! -d $BUILD_DIR ]; then
		fcho "Error: Build directory not detected, please run -ubd, or -b before deploying"
		exit 0
	fi
	mkdir $DEPLOY_DIR
	cp -r $BUILD_DIR/qTox.app $DEPLOY_DIR/qTox.app
	cd $DEPLOY_DIR
	fcho "Now working in ${PWD}"
	$MACDEPLOYQT qTox.app
}

# The commands
if [ "$1" == "-i" ]; then
	install
	exit 0
fi
	
if [ "$1" == "-u" ]; then
	update
	exit 0
fi

if [ "$1" == "-b" ]; then
	build
	exit 0
fi

if [ "$1" == "-d" ]; then
	deploy
	exit 0
fi

if [ "$1" == "-ubd" ]; then
	update
	build
	deploy
	exit 0
fi

if [ "$1" == "-h" ]; then
	echo "This script was created to help ease the process of compiling and creating a distribuable qTox package for OSX systems."
	echo "The avilable commands are:"
	echo "-h -- This help text."
	echo "-i -- A slightly automated process for getting an OSX machine ready to build Toxcore and qTox."
	echo "-u -- Check for updates and build Toxcore from git & update qTox from git."
	echo "-b -- Builds qTox in: ${BUILD_DIR}"
	echo "-d -- Makes a distributeable qTox.app file in: ${DEPLOY_DIR}"
	echo "-ubd -- Does -u, -b, and -d sequentially"
	fcho "Issues with Toxcore or qTox should be reported to their respective repos: https://github.com/irungentoo/toxcore | https://github.com/tux3/qTox"
	exit 0
fi

fcho "Oh dear! You seemed to of started this script improperly! Use -h to get avilable commands and information!" 
exit 0