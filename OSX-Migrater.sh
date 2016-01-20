#!/usr/bin/env bash

# A qTox profile migrater for OSX
now=$(date +"%m_%d_%Y-%H.%M.%S")
bak="~/.Tox-Backup-$now"

echo "Figuring out if action is required ..."
if [ -d ~/Library/Preferences/tox ]; then
	echo "Moving profile(s) ..."
	cp -r ~/Library/Preferences/tox ~/Library/Application\ Support/
	mv ~/Library/Application\ Support/tox/ ~/Library/Application\ Support/Tox
	mv ~/Library/Preferences/tox ~/.Tox-Backup-$now
	echo "Done! You profile(s) have been moved! A back up coppy still exists at:"
	echo "$bak"
else
	echo "Cannot locate old profile directory, profile migration not performed"
fi
exit 0