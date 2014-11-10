#!/bin/bash
echo -n 1 > /var/www/html/qtox/win32/version
./qtox-updater-sign $1 >> /var/www/html/qtox/win32/version

