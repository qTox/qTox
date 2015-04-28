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
if [ ! -f "libtoxcore-win32-i686.zip" ]; then
   wget --no-check-certificate http://jenkins.libtoxcore.so/job/libtoxcore-win32-i686/lastSuccessfulBuild/artifact/libtoxcore-win32-i686.zip
   rm -rf include/tox
fi

if [ ! -d "include/tox" ]; then
   $QTOX_DIR/tools/unzip -o libtoxcore-win32-i686.zip -d ./
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

## opencv
if [ ! -f "opencv-2.4.9.tar.gz" ]; then
   wget --no-check-certificate https://github.com/Itseez/opencv/archive/2.4.9.tar.gz -O opencv-2.4.9.tar.gz
   rm -rf opencv-2.4.9
fi

if [ ! -d "opencv-2.4.9" ]; then
   tar -xvf opencv-2.4.9.tar.gz
   rm bin/libopencv_core249.dll
fi

if [ ! -f "bin/libopencv_core249.dll" ]; then
   mkdir opencv-2.4.9/build
   pushd opencv-2.4.9/build
   cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$QTOX_DIR/libs \
       -DBUILD_opencv_apps=NO \
       -DBUILD_opencv_calib3d=NO \
       -DBUILD_opencv_contrib=NO \
       -DBUILD_opencv_core=YES \
       -DBUILD_opencv_features2d=NO \
       -DBUILD_opencv_flann=NO \
       -DBUILD_opencv_gpu=NO \
       -DBUILD_opencv_highgui=YES \
       -DBUILD_opencv_imgproc=YES \
       -DBUILD_opencv_legacy=NO \
       -DBUILD_opencv_ml=NO \
       -DBUILD_opencv_nonfree=NO \
       -DBUILD_opencv_objdetect=NO \
       -DBUILD_opencv_ocl=NO \
       -DBUILD_opencv_photo=NO \
       -DBUILD_opencv_stiching=NO \
       -DBUILD_opencv_superres=NO \
       -DBUILD_opencv_ts=NO \
       -DBUILD_opencv_video=NO \
       -DBUILD_opencv_videostab=NO \
       -DBUILD_opencv_world=NO \
       -DWITH_QT=NO \
       -DBUILD_EXAMPLES=NO \
       ..

   make
   make install
   for arch in x86 x64
   do
       if [ -d $QTOX_DIR/libs/$arch/mingw ]; then
           mv $QTOX_DIR/libs/$arch/mingw/bin/* $QTOX_DIR/libs/bin/
           mv $QTOX_DIR/libs/$arch/mingw/lib/* $QTOX_DIR/libs/lib/
           rm -rf $QTOX_DIR/libs/$arch
       fi
   done
   popd
fi

echo **Done**
