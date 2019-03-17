#!/usr/bin/env bash

set -e -u pipefail

readonly SCRIPT_DIR=$(dirname $(readlink -f $0))

if [[ $(id -u) != 0 ]]
then
    >&2 echo "Please run as root."
    exit 1
fi

if [[ -z $(which apparmor_parser) ]]
then
   >&2 echo "AppArmor not found."
   exit 1
fi

#NOTE: we do not need to create /etc/apparmor.d/tunables/usr.bin.qtox.d/ or
#/etc/apparmor.d/local/usr.bin.qtox because AppArmor >2.13 support #include if
#exists

echo "Copying AppArmor files..."
cp -v "${SCRIPT_DIR}/tunables/usr.bin.qtox" "/etc/apparmor.d/tunables/"
cp -v "${SCRIPT_DIR}/usr.bin.qtox" "/etc/apparmor.d/"

echo "Restarting AppArmor..."
systemctl restart apparmor

echo "Done."

