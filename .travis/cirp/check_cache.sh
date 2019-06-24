#!/usr/bin/env bash

if [ -f "/opt/cirp/previous_runs_commit" ] && [ "$(cat /opt/cirp/previous_runs_commit)" == "$(git rev-parse HEAD)" ]
then
  # No new commits in the repo
  touch /opt/cirp/previous_runs_commit
  git log -1
  echo "No new commits in the repo"
  exit 0
fi
