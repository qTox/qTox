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
	MAIN_DIR="${TRAVIS_BUILD_DIR}"
	QTOX_DIR="${MAIN_DIR}"
else
	MAIN_DIR="/Users/${USER}"
	QTOX_DIR="${MAIN_DIR}/qTox"
fi
QT_DIR="/usr/local/Cellar/qt5" # Folder name of QT install
# Figure out latest version
QT_VER=($(ls ${QT_DIR} | sed -n -e 's/^\([0-9]*\.([0-9]*\.([0-9]*\).*/\1/' -e '1p;$p'))
QT_DIR_VER="${QT_DIR}/${QT_VER[1]}"

QMAKE="${QT_DIR_VER}/bin/qmake" # Don't change
MACDEPLOYQT="${QT_DIR_VER}/bin/macdeployqt" # Don't change

TOXCORE_DIR="${MAIN_DIR}/toxcore" # Change to Git location

FA_DIR="${MAIN_DIR}/filter_audio"
LIB_INSTALL_PREFIX="${QTOX_DIR}/libs"

if [[ ! -e "${LIB_INSTALL_PREFIX}" ]]; then
	mkdir -p "${LIB_INSTALL_PREFIX}"
fi

BUILD_DIR="${MAIN_DIR}/qTox-Mac_Build"
DEPLOY_DIR="${MAIN_DIR}/qTox-Mac_Deployed"


function fcho() {
	local msg="$1"; shift
	printf "\n$msg\n" "$@"
}

function build_toxcore() {
	echo "Starting Toxcore build and install"
	cd $TOXCORE_DIR
	echo "Now working in: ${PWD}"
	
	local LS_DIR="/usr/local/Cellar/libsodium/"
	#Figure out latest version
	local LS_VER=($(ls ${LS_DIR} | sed -n -e 's/^\([0-9]*\.([0-9]*\.([0-9]*\).*/\1/' -e '1p;$p'))
	local LS_DIR_VER="${LS_DIR}/${LS_VER[1]}"
	
	sleep 3
	
	autoreconf -if
	
	#Make sure the correct version of libsodium is used
	./configure --with-libsodium-headers="${LS_DIR_VER}/include/" --with-libsodium-libs="${LS_DIR_VER}/lib/" --prefix="${LIB_INSTALL_PREFIX}"
	
	make clean &> /dev/null
	fcho "Compiling toxcore."
	make > /dev/null || exit 1
	fcho "Installing toxcore."
	make install > /dev/null || exit 1
}

function install() {
	fcho "=============================="
	fcho "This script will install the necessary applications and libraries needed to compile qTox properly."
	fcho "Note that this is not a 100 percent automated install it just helps simplify the process for less experienced or lazy users."
	if [[ $TRAVIS = true ]]; then #travis check
		echo "Oh... It's just Travis...."
	else
	read -n1 -rsp $'Press any key to continue or Ctrl+C to exit...\n'
	fi
		
	if [[ -e /usr/local/bin/brew ]]; then
		fcho "Homebrew already installed!"
	else
		fcho "Installing homebrew ..."
		ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
	fi
	fcho "Updating brew formulas ..."
	if [[ $TRAVIS = true ]]; then
		brew update > /dev/null
	else
		brew update
	fi
	fcho "Getting home brew formulas ..."
	sleep 3
	if [[ $TRAVIS != true ]]; then #travis check
		brew install wget libtool automake
	fi
	brew install git ffmpeg qrencode autoconf check qt5 libvpx opus sqlcipher libsodium
	
	QT_VER=($(ls ${QT_DIR} | sed -n -e 's/^\([0-9]*\.([0-9]*\.([0-9]*\).*/\1/' -e '1p;$p'))
	QT_DIR_VER="${QT_DIR}/${QT_VER[1]}"
	
	fcho "Installing x-code Command line tools ..."
	xcode-select --install
	
	fcho "Starting git repo checks ..."
	
	cd $MAIN_DIR # just in case
	# Toxcore
	if [[ -e $TOX_DIR/.git/index ]]; then # Check if this exists
		fcho "Toxcore git repo already in place !"
		cd $TOX_DIR
		git pull
	else
		fcho "Cloning Toxcore git ... "
		git clone https://github.com/irungentoo/toxcore.git
	fi
	# qTox
	if [[ $TRAVIS = true ]]; then #travis check
		fcho "Travis... You already have qTox..."
	else
		if [[ -e $QTOX_DIR/.git/index ]]; then # Check if this exists
			fcho "qTox git repo already in place !"
			cd $QTOX_DIR
			git pull
		else
			fcho "Cloning qTox git ... "
			git clone https://github.com/tux3/qTox.git
		fi
	fi
	# filter_audio
	if [[ -e $FA_DIR/.git/index ]]; then # Check if this exists
		fcho "Filter_Audio git repo already in place !"
		cd $FA_DIR
		git pull
	else
		fcho "Cloning Filter_Audio git ... "
		git clone https://github.com/irungentoo/filter_audio.git
		cd $FA_DIR
	fi
	fcho "Installing filter_audio."
	make install PREFIX="${LIB_INSTALL_PREFIX}"
	
	# toxcore build
	if [[ $TRAVIS = true ]]; then #travis check
		build_toxcore
	else
		fcho "If all went well you should now have all the tools needed to compile qTox!"
		read -r -p "Would you like to install toxcore now? [y/N] " response
		if [[ $response =~ ^([yY][eE][sS]|[yY])$ ]]; then
			build_toxcore
			
		else
		    fcho "You can simply use the -u command and say [Yes/n] when prompted"
		fi
	fi
	# put required by qTox libs/headers in `libs/`
	cd "${QTOX_DIR}"
	sudo ./bootstrap-osx.sh
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
		build_toxcore
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
		sudo ./bootstrap-osx.sh
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

function bootstrap() {
	fcho "------------------------------"
	fcho "starting bootstrap process ..."
	
	# filter_audio
	cd $FA_DIR
	fcho "Installing filter_audio."
	make install PREFIX="${LIB_INSTALL_PREFIX}"
	
	#Toxcore
	build_toxcore
	
	#Boot Strap
	fcho "Running: sudo ${QTOX_DIR_VER}/bootstrap-osx.sh"
	cd $QTOX_DIR
	sudo ./bootstrap-osx.sh
}

# The commands
if [[ "$1" == "-i" ]]; then
	install
	exit
fi
	
if [[ "$1" == "-u" ]]; then
	update
	exit
fi

if [[ "$1" == "-b" ]]; then
	build
	exit
fi

if [[ "$1" == "-boot" ]]; then
	bootstrap
	exit
fi

if [[ "$1" == "-d" ]]; then
	deploy
	exit
fi

if [[ "$1" == "-ubd" ]]; then
	update
	build
	deploy
	exit
fi

if [[ "$1" == "-h" ]]; then
	echo "This script was created to help ease the process of compiling and creating a distributable qTox package for OSX systems."
	echo "The available commands are:"
	echo "-h -- This help text."
	echo "-i -- A slightly automated process for getting an OSX machine ready to build Toxcore and qTox."
	echo "-u -- Check for updates and build Toxcore from git & update qTox from git."
	echo "-b -- Builds qTox in: ${BUILD_DIR}"
	echo "-boot -- Performs bootstrap steps."
	echo "-d -- Makes a distributable qTox.app file in: ${DEPLOY_DIR}"
	echo "-ubd -- Does -u, -b, and -d sequentially"
	fcho "Issues with Toxcore or qTox should be reported to their respective repos: https://github.com/irungentoo/toxcore | https://github.com/tux3/qTox"
	exit 0
fi

fcho "Oh dear! You seemed to of started this script improperly! Use -h to get available commands and information!"
echo " "
say -v Kathy -r 255 "Oh dear! You seemed to of started this script improperly! Use -h to get available commands and information!"
exit 0
