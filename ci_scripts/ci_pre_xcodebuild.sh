#!/bin/sh
env

echo ""
echo "Workspace"
ls -lart ${CI_WORKSPACE_PATH}

echo ""
echo "Archive"
ls -lart ${CI_ARCHIVE_PATH}

echo ""
echo "TMP"
ls -lart ${TMPDIR}


mkdir -p $CI_DERIVED_DATA_PATH
cd $CI_DERIVED_DATA_PATH
ls -lart
if [ ! -d xLights-macOS-dependencies ]; then
    git clone --depth 1 https://github.com/xLightsSequencer/xLights-macOS-dependencies.git xLights-macOS-dependencies
fi
cd xLights-macOS-dependencies
git reset --hard
git pull
cd ..
echo ""
echo "Post update"
ls -lart


cd ${CI_PRIMARY_REPOSITORY_PATH}/macOS
rm -rf dependencies
ln -s ${CI_DERIVED_DATA_PATH}/xLights-macOS-dependencies dependencies
echo ""
echo "Post link"
ls -lart
