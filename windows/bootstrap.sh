#!/bin/sh
QTOX_DIR=`pwd`/..

if [ -d $QTOX_DIR/libs ]; then
    echo "Remove ./libs and redownload/recompile dependencies?"
    read -p "m/a/N (missing/all/no): " yn
    yn=$(echo $yn | tr "[:upper:]" "[:lower:]")
    if [ "$yn" == "a" ]; then
        rm -rf $QTOX_DIR/libs
    elif [ "$yn" == "n" ]; then
        exit -1
    elif [ "$yn" != "m" ]; then
        exit -1
    fi
fi

mkdir -p $QTOX_DIR/libs
cd $QTOX_DIR/libs


## toxcore
if [ ! -f "libtoxcore_build_windows_x86_shared_release.zip" ]; then
    wget --no-check-certificate https://build.tox.chat/view/libtoxcore/job/libtoxcore_build_windows_x86_shared_release/lastSuccessfulBuild/artifact/libtoxcore_build_windows_x86_shared_release.zip
    rm -rf include/tox
fi

if [ ! -d "include/tox" ]; then
    unzip -o libtoxcore_build_windows_x86_shared_release.zip -d ./
fi


## filter_audio
if [ ! -d $QTOX_DIR/libs/filter_audio ]; then
    git clone https://github.com/irungentoo/filter_audio.git $QTOX_DIR/libs/filter_audio
    rm bin/libfilteraudio.dll
else
    pushd $QTOX_DIR/libs/filter_audio
    git pull
    popd
fi

if [ ! -f "bin/libfilteraudio.dll" ]; then
    pushd $QTOX_DIR/libs/filter_audio
    PREFIX="$QTOX_DIR/libs" CC="gcc.exe" make install
    mv libfilteraudio.dll.a $QTOX_DIR/libs/lib
    popd
    if [ -f "lib/libfilteraudio.dll" ]; then
        mv lib/libfilteraudio.dll bin/
    fi
fi


## qrencode
if [ ! -f "qrencode-3.4.4.tar.gz" ]; then
    wget http://fukuchi.org/works/qrencode/qrencode-3.4.4.tar.gz
fi

if [ ! -d "$QTOX_DIR/libs/qrencode-3.4.4" ]; then
    tar -xvf qrencode-3.4.4.tar.gz
    rm -rf lib/qrcodelib.dll
fi

if [ ! -f "lib/libqrencode.a" ]; then
    pushd $QTOX_DIR/libs/qrencode-3.4.4
    ./autogen.sh
    ./configure --prefix=$QTOX_DIR/libs --without-tests --without-libiconv-prefix --without-tools
    make
    make install
    popd
fi


## OpenAL
if [ ! -f "openal-soft-1.16.0.tar.bz2" ]; then
    wget http://kcat.strangesoft.net/openal-releases/openal-soft-1.16.0.tar.bz2
    rm -rf openal-soft-1.16.0
fi

if [ ! -d "openal-soft-1.16.0" ]; then
    tar -xvf openal-soft-1.16.0.tar.bz2
    rm bin/OpenAL32.dll
fi

if [ ! -f "bin/OpenAL32.dll" ]; then
    pushd openal-soft-1.16.0/build
    cmake -G "MSYS Makefiles" -DQT_QMAKE_EXECUTABLE=NOTFOUND -DCMAKE_BUILD_TYPE=Release -DALSOFT_REQUIRE_DSOUND=NO -DCMAKE_INSTALL_PREFIX=$QTOX_DIR/libs ..
    make
    make install
    popd
fi


## ffmpeg
if [ ! -f "ffmpeg-2.7.tar.bz2" ]; then
    wget http://ffmpeg.org/releases/ffmpeg-2.7.tar.bz2 -O ffmpeg-2.7.tar.bz2
    rm -rf ffmpeg-2.7
fi

if [ ! -d "ffmpeg-2.7" ]; then
    tar -xvf ffmpeg-2.7.tar.bz2
    rm bin/avcodec-56.dll
    pushd ffmpeg-2.7
    patch -p1 < $QTOX_DIR/windows/ffmpeg-2.7-mingw.diff
    popd
fi

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

echo **Done**
