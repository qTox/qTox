#!/bin/bash
echo -n 2 > version
./qtox-updater-sign `date +%s`!$1 >> version

