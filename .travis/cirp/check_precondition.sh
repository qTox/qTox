#!/usr/bin/env bash

if [ ! -z "$TRAVIS_EVENT_TYPE" ] && [ "$TRAVIS_EVENT_TYPE" != "cron" ]
then
  echo "Skipping publishing in a non-cron build"
  exit 0
fi

if [ ! -z "$TRAVIS_PULL_REQUEST" ] && [ "$TRAVIS_PULL_REQUEST" != "false" ]
then
  echo "Skipping publishing in a Pull Request"
  exit 0
fi
