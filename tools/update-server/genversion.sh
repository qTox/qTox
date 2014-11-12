#!/bin/bash
echo -n 1 > version
./qtox-updater-sign $1 >> version

