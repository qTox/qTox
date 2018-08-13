#!/bin/bash

cd "$(dirname "$0")/.."
docker build . -f docker/Dockerfile.ubuntu -t qtox
cd -
