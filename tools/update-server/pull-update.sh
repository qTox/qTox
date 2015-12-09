#!/bin/bash

# win64 update
rm -f *.zip*
rm -f gitversion
rm -rf win64/*
mkdir -p win64
wget https://build.tox.chat/view/qtox/job/qTox-stable_build_windows_x86-64_release/lastSuccessfulBuild/artifact/qTox-stable_build_windows_x86-64_release.zip
wget https://build.tox.chat/view/qtox/job/qTox-stable_build_windows_x86-64_release/lastSuccessfulBuild/artifact/version -O gitversion
unzip -o qTox-stable_build_windows_x86-64_release.zip -d win64/source

echo -n 3 > win64/version
qtox-updater-sign `date +%s`!`cat gitversion` >> win64/version
qtox-updater-genflist win64

rm -rf win64/source
rm -f /var/www/html/qtox/win64/version
rm -rf /var/www/html/qtox/win64/*
cp -r win64/* /var/www/html/qtox/win64/

# win32 update, I don't feel like writing bash functions edition
rm -f *.zip*
rm -f gitversion
rm -rf win32/*
mkdir -p win32
wget https://build.tox.chat/view/qtox/job/qTox-stable_build_windows_x86_release/lastSuccessfulBuild/artifact/qTox-stable_build_windows_x86_release.zip
wget https://build.tox.chat/view/qtox/job/qTox-stable_build_windows_x86_release/lastSuccessfulBuild/artifact/version -O gitversion
unzip -o qTox-stable_build_windows_x86_release.zip -d win32/source

echo -n 3 > win32/version
qtox-updater-sign `date +%s`!`cat gitversion` >> win32/version
qtox-updater-genflist win32

rm -rf win32/source
rm -f /var/www/html/qtox/win32/version
rm -rf /var/www/html/qtox/win32/*
cp -r win32/* /var/www/html/qtox/win32/
