#!/bin/sh
env
pwd

cd $CI_DERIVED_DATA_PATH
if [ ! -d xLights-macOS-dependencies.git ]; then
    git clone https://github.com/xLightsSequencer/xLights-macOS-dependencies.git
fi
cd xLights-macOS-dependencies.git
git reset --hard
git pull

cd ${CI_PRIMARY_REPOSITORY_PATH}/macOS
rm -rf dependencies
ln -s ${$CI_DERIVED_DATA_PATH}/xLights-macOS-dependencies.git dependencies
