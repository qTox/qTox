#!/bin/sh

# use this to enable/disable debug output
readonly ENABLE_DEBUG_OUTPUT=false

QTOX_DIR=`pwd`/..

# Generates a "Press any key to continue..."-Message
pause() {
    read -n1 -r -p "Press any key to continue..."
}

# Writes out debug output lines
debug_out() {
    if [[ $ENABLE_DEBUG_OUTPUT = "true" ]]
    then
        echo
        echo "   ###   $1"
        echo
    fi
}
debug_out "Debug output enabled."

echo

# ask how to proceed
if [ -d $QTOX_DIR/libs ]; then
    echo "Remove ./libs and redownload/recompile dependencies?"
    read -p "m/a/N (missing/all/no): " read_input
    read_input=$(echo $read_input | tr "[:upper:]" "[:lower:]")
    if [ "$read_input" == "a" ]; then
        debug_out "Parameter is a: Deleting existing libraries in ./libs ..."
        rm -rf $QTOX_DIR/libs
    elif [ "$read_input" == "n" ]; then
        debug_out "Parameter is n: Exiting with -1."
        exit -1
    elif [ "$read_input" == "m" ]; then
        debug_out "Parameter is m: Continuing without removing ./libs ..."
    else
        debug_out "Input invalid. Exiting with -1."
        exit -1
    fi
fi

echo
echo

debug_out "Creating DIR: $QTOX_DIR/libs"
mkdir -p $QTOX_DIR/libs
cd $QTOX_DIR/libs


## toxcore
debug_out "toxcore: Obtaining latest compiled release."
if [ ! -f "libtoxcore_build_windows_x86_shared_release.zip" ]; then
    wget --no-check-certificate https://build.tox.chat/view/libtoxcore/job/libtoxcore_build_windows_x86_shared_release/lastSuccessfulBuild/artifact/libtoxcore_build_windows_x86_shared_release.zip
    rm -rf include/tox
fi

debug_out "toxcore: Unpacking build includes and binaries."
if [ ! -d "include/tox" ]; then
    unzip -o libtoxcore_build_windows_x86_shared_release.zip -d ./
fi


## libqrencode
debug_out "libqrencode: Obtaining version 3.4.4"
if [ ! -f "qrencode-3.4.4.tar.gz" ]; then
    wget https://fukuchi.org/works/qrencode/qrencode-3.4.4.tar.gz
fi

if [ ! -f "qrencode-3.4.4.tar.gz" ]; then
    echo
    echo
    echo "Warning: Could not download libqrencode from fukuchi.org."
    echo "Warning: In the past, there have been certificate issues on that server."
    echo "Warning: Trying again while ignoring certificate errors."
    echo
    echo "Warning: This is highly INSECURE."
    echo
    echo
    echo "Do you want to continue?"
    read -p "y/N (yes/no): " read_input
    read_input=$(echo $read_input | tr "[:upper:]" "[:lower:]")
    if [ "$read_input" == "y" ]; then
        debug_out "libqrencode: Obtaining version 3.4.4 without certificate check."
        wget --no-check-certificate https://fukuchi.org/works/qrencode/qrencode-3.4.4.tar.gz
    else
        echo "libqrencode: Download canceled."
        echo "Continuing with next step."
        echo
    fi
fi

debug_out "libqrencode: Unpacking"
if [ ! -d "$QTOX_DIR/libs/qrencode-3.4.4" ]; then
    tar -xvf qrencode-3.4.4.tar.gz
    rm -rf lib/qrcodelib.dll
fi

debug_out "libqrencode: Configuring and compiling"
if [ ! -f "lib/libqrencode.a" ]; then
    pushd $QTOX_DIR/libs/qrencode-3.4.4
    ./autogen.sh
    ./configure --prefix=$QTOX_DIR/libs --without-tests --without-libiconv-prefix --without-tools
    make
    make install
    popd
fi


## OpenAL
debug_out "OpenAL: Obtaining version 1.16.0"
if [ ! -f "openal-soft-1.16.0.tar.bz2" ]; then
    wget http://kcat.strangesoft.net/openal-releases/openal-soft-1.16.0.tar.bz2
    rm -rf openal-soft-1.16.0
fi

debug_out "OpenAL: Unpacking"
if [ ! -d "openal-soft-1.16.0" ]; then
    tar -xvf openal-soft-1.16.0.tar.bz2
    rm bin/OpenAL32.dll
fi

debug_out "OpenAL: Configuring and compiling"
if [ ! -f "bin/OpenAL32.dll" ]; then
    pushd openal-soft-1.16.0/build
    CFLAGS="-D_TIMESPEC_DEFINED" cmake -G "MSYS Makefiles" -DQT_QMAKE_EXECUTABLE=NOTFOUND -DCMAKE_BUILD_TYPE=Release -DALSOFT_REQUIRE_DSOUND=NO -DCMAKE_INSTALL_PREFIX=$QTOX_DIR/libs ..
    make
    make install
    popd
fi


## ffmpeg
debug_out "ffmpeg: Obtaining version 2.7"
if [ ! -f "ffmpeg-2.7.tar.bz2" ]; then
    wget http://ffmpeg.org/releases/ffmpeg-2.7.tar.bz2 -O ffmpeg-2.7.tar.bz2
    rm -rf ffmpeg-2.7
fi

debug_out "ffmpeg: Unpacking and patching for mingw"
if [ ! -d "ffmpeg-2.7" ]; then
    tar -xvf ffmpeg-2.7.tar.bz2
    rm bin/avcodec-56.dll
    pushd ffmpeg-2.7
    patch -p1 < $QTOX_DIR/windows/ffmpeg-2.7-mingw.diff
    popd
fi

debug_out "ffmpeg: Configuring and compiling"
if [ ! -f "bin/avcodec-56.dll" ]; then
    pushd ffmpeg-2.7
    ./configure --target-os=mingw32 --prefix=$QTOX_DIR/libs \
        --enable-memalign-hack --disable-swscale-alpha --disable-programs --disable-doc --disable-postproc \
        --disable-avfilter --disable-avresample --disable-swresample --disable-protocols --disable-filters \
        --disable-network --disable-muxers --disable-sdl --disable-iconv --disable-bzlib --disable-lzma \
        --disable-zlib --disable-xlib --disable-encoders --enable-shared --disable-static --disable-yasm
    make
    make install
    popd
fi

echo
echo "Setting Environment Variable: PKG_CONFIG_PATH"
setx PKG_CONFIG_PATH $QTOX_DIR/libs/lib/pkgconfig

echo 
echo Library setup finised!
echo
