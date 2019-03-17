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

echo "Copying AppArmor files..."
[[ ! -d "/etc/apparmor.d/tunables/usr.bin.qtox.d/" ]] && mkdir -v "/etc/apparmor.d/tunables/usr.bin.qtox.d/"
cp -v "${SCRIPT_DIR}/tunables/usr.bin.qtox" "/etc/apparmor.d/tunables/"
cp -v "${SCRIPT_DIR}/usr.bin.qtox" "/etc/apparmor.d/"
touch "/etc/apparmor.d/local/usr.bin.qtox"

echo "Restarting AppArmor..."
systemctl restart apparmor

echo "Done."

