#!/bin/sh
env
pwd

cd ${CI_PRIMARY_REPOSITORY_PATH}/macOS
rm -rf dependencies
git clone https://github.com/xLightsSequencer/xLights-macOS-dependencies.git dependencies
