#!/usr/bin/env bash

# MIT License
#
# Copyright (c) 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

# Known issues:
# - Doesn't build qTox updater, because it wasn't ported to cmake yet and
#   because it requires static Qt, which means we'd need to build Qt twice, and
#   building Qt takes really long time.


set -euo pipefail

RUN_TESTS=0

while (( $# > 0 )); do
    case $1 in
        --src-dir) QTOX_SRC_DIR=$2; shift 2 ;;
        --arch) ARCH=$2; shift 2 ;;
        --run-tests) RUN_TESTS=1; shift ;;
        --build-type) BUILD_TYPE=$2; shift 2;;
        *) "Unexpected argument $1"; exit 1 ;;
    esac
done

# Common directory paths

QTOX_PREFIX_DIR="$(realpath install-prefix)"
readonly QTOX_PREFIX_DIR
QTOX_PACKAGE_DIR="$(realpath package-prefix)"
readonly QTOX_PACKAGE_DIR


if [ -z "$ARCH" ]
then
  echo "Error: No architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the first argument to the script."
  exit 1
fi

if [[ "$ARCH" != "i686" ]] && [[ "$ARCH" != "x86_64" ]]
then
  echo "Error: Incorrect architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the first argument to the script."
  exit 1
fi

if [ -z "$QTOX_SRC_DIR" ]; then
  echo "--src-dir must be specified"
fi

if [ -z "$BUILD_TYPE" ]
then
  echo "Error: No build type was specified. Please specify either 'release' or 'debug', case sensitive, as the second argument to the script."
  exit 1
fi

if [[ "$BUILD_TYPE" != "release" ]] && [[ "$BUILD_TYPE" != "debug" ]]
then
  echo "Error: Incorrect build type was specified. Please specify either 'release' or 'debug', case sensitive, as the second argument to the script."
  exit 1
fi

# Spell check on windows currently not supported, disable
if [[ "$BUILD_TYPE" == "release" ]]
then
  cmake -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
        -DCMAKE_PREFIX_PATH=/windows \
        -DCMAKE_BUILD_TYPE=Release \
        -DSPELL_CHECK=OFF \
        -DDESKTOP_NOTIFICATIONS=ON \
        -DUPDATE_CHECK=ON \
        -DSTRICT_OPTIONS=ON \
        -DTEST_CROSSCOMPILING_EMULATOR=wine \
        "$QTOX_SRC_DIR"
elif [[ "$BUILD_TYPE" == "debug" ]]
then
  cmake -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
        -DCMAKE_PREFIX_PATH=/windows \
        -DCMAKE_BUILD_TYPE=Debug \
        -DSPELL_CHECK=OFF \
        -DDESKTOP_NOTIFICATIONS=ON \
        -DUPDATE_CHECK=ON \
        -DSTRICT_OPTIONS=ON \
        -DTEST_CROSSCOMPILING_EMULATOR=wine \
        "$QTOX_SRC_DIR"
fi

make -j $(nproc)

mkdir -p "${QTOX_PREFIX_DIR}"
cp qtox.exe $QTOX_PREFIX_DIR
cp -r /export/* $QTOX_PREFIX_DIR

# Run tests
set +u
if [[ $RUN_TESTS -ne 0 ]]
then
  export WINEPATH='/export;/windows/bin'
  export CTEST_OUTPUT_ON_FAILURE=1
  export PATH="$PATH:/opt/wine-stable/bin"
  ctest -j$(nproc)
fi
set -u

# Strip
set +e
if [[ "$BUILD_TYPE" == "release" ]]
then
  $ARCH-w64-mingw32-strip -s $QTOX_PREFIX_DIR/*.exe
fi
$ARCH-w64-mingw32-strip -s $QTOX_PREFIX_DIR/*.dll
$ARCH-w64-mingw32-strip -s $QTOX_PREFIX_DIR/*/*.dll
set -e

if [[ "$BUILD_TYPE" == "debug" ]]
then
  cp -r /debug_export/* ${QTOX_PREFIX_DIR}
fi

# Create zip
pushd $QTOX_PREFIX_DIR
zip qtox-"$ARCH"-"$BUILD_TYPE".zip -r *
popd

# Create installer
if [[ "$BUILD_TYPE" == "release" ]]
then
  mkdir -p $QTOX_PACKAGE_DIR
  pushd $QTOX_PACKAGE_DIR
  # The installer creation script expects all the files to be in qtox/*
  mkdir -p qtox
  cp -r $QTOX_PREFIX_DIR/* ./qtox
  rm ./qtox/*.zip

  cp -r $QTOX_SRC_DIR/windows/* .
  # Select the installer script for the correct architecture
  if [[ "$ARCH" == "i686" ]]
  then
    makensis qtox.nsi
  elif [[ "$ARCH" == "x86_64" ]]
  then
    makensis qtox64.nsi
  fi

  popd
fi

# dll check
# Create lists of all .exe and .dll files
find "$QTOX_PREFIX_DIR" -iname '*.dll' > dlls
find "$QTOX_PREFIX_DIR" -iname '*.exe' > exes

# Create a list of dlls that are loaded during the runtime (not listed in the PE
# import table, thus ldd doesn't print those)
echo "$QTOX_PREFIX_DIR/libsnore-qt5/libsnore_backend_windowstoast.dll
$QTOX_PREFIX_DIR/iconengines/qsvgicon.dll
$QTOX_PREFIX_DIR/imageformats/qgif.dll
$QTOX_PREFIX_DIR/imageformats/qico.dll
$QTOX_PREFIX_DIR/imageformats/qjpeg.dll
$QTOX_PREFIX_DIR/imageformats/qsvg.dll
$QTOX_PREFIX_DIR/platforms/qdirect2d.dll
$QTOX_PREFIX_DIR/platforms/qminimal.dll
$QTOX_PREFIX_DIR/platforms/qoffscreen.dll
$QTOX_PREFIX_DIR/platforms/qwindows.dll" > runtime-dlls
if [[ "$ARCH" == "i686" ]]
then
  echo "$QTOX_PREFIX_DIR/libssl-1_1.dll" >> runtime-dlls
elif [[ "$ARCH" == "x86_64" ]]
then
  echo "$QTOX_PREFIX_DIR/libssl-1_1-x64.dll" >> runtime-dlls
fi

# Create a tree of all required dlls
# Assumes all .exe files are directly in $QTOX_PREFIX_DIR, not in subdirs
while IFS= read -r line
do
  if [[ "$ARCH" == "i686" ]]
  then
    WINE_DLL_DIR="/opt/wine-stable/lib/wine/i386-windows"
  elif [[ "$ARCH" == "x86_64" ]]
  then
    WINE_DLL_DIR="/opt/wine-stable/lib64/wine/x86_64-windows /opt/wine-stable/lib/wine/i386-windows"
  fi
  python3 /usr/local/bin/mingw-ldd.py $line --dll-lookup-dirs $QTOX_PREFIX_DIR $WINE_DLL_DIR --output-format tree >> dlls-required
done < <(cat exes runtime-dlls)

# Check that no extra dlls get bundled
while IFS= read -r line
do
  if ! grep -q "$line" dlls-required
  then
    echo "Error: extra dll included: $line. If this is a mistake and the dll is actually needed (e.g. it's loaded at runtime), please add it to the runtime dll list."
    exit 1
  fi
done < dlls

# Check that no dll is missing
if grep -q 'not found' dlls-required
then
  cat dlls-required
  echo "Error: Missing some dlls."
  exit 1
fi

# Check that OpenAL is bundled. It is availabe from WINE, but not on Windows systems
if grep -q '/opt/wine-stable/lib/wine/i386-windows/openal32.dll' dlls-required
then
  cat dlls-required
  echo "Error: Missing OpenAL."
  exit 1
fi
