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

if [ "$CI_PRIMARY_REPOSITORY_PATH" = "" ]; then
    CI_PRIMARY_REPOSITORY_PATH=`pwd`
fi

if [ "$CI_DERIVED_DATA_PATH" = "" ]; then
    CI_DERIVED_DATA_PATH=`pwd`/macOS
fi

cd $CI_PRIMARY_REPOSITORY_PATH
git submodule update --init

mkdir -p $CI_DERIVED_DATA_PATH
cd $CI_DERIVED_DATA_PATH
ls -lart

#install zstd so we can decompress the deps
brew install zstd


# need to install the metal toolchain, issue with Xcode 26 in Xcode Cloud
echo "Downloading and importing Metal Toolchain..."
xcodebuild -downloadComponent metalToolchain -exportPath /tmp/MyMetalExport/
xcodebuild -importComponent metalToolchain -importPath /tmp/MyMetalExport/MetalToolchain-*.exportedBundle
echo "Metal Toolchain imported successfully."
