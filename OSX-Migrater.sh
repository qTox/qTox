#!/usr/bin/env bash

# A qTox profile migrater for OSX
echo "Figuring out if action is required ..."
if [ -d ~/Library/Prefrences/tox]
	echo "Moving profile(s) ..."
	cp -r ~/Library/Preferences/tox ~/Library/Application\ Support/
	mv ~/Library/Application\ Support/tox/ ~/Library/Application\ Support/Tox
	mv ~/Library/Preferences/tox ~/.Tox-Backup
	echo "Done! You profile(s) have been moved! A back up coppy still exists at:"
	echo "~/.Tox-Backup"
else
	echo "Cannot locate old profile directory, profile migration not performed"
fi
exit 0