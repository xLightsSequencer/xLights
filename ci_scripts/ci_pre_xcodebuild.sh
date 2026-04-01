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


if xcrun -find metal >/dev/null 2>&1; then
    echo "Metal tools are already installed."
else
    echo "Metal tools are NOT installed."
    xcodebuild -downloadComponent metalToolchain -exportPath /tmp/MyMetalExport/
    xcodebuild -importComponent metalToolchain -importPath /tmp/MyMetalExport/MetalToolchain-*.exportedBundle

    if xcrun -find metal >/dev/null 2>&1; then
        echo "Metal tools were successfully installed."
    else
        echo "Metal tools are NOT installed. Trying again."
        xcodebuild -downloadComponent metalToolchain -exportPath /tmp/MyMetalExport/
        xcodebuild -importComponent metalToolchain -importPath /tmp/MyMetalExport/MetalToolchain-*.exportedBundle

        if xcrun -find metal >/dev/null 2>&1; then
            echo "Metal tools were successfully installed."
        else
            echo "Standard Metal Toolchain install failed, falling back to manual download..."
            mkdir -p /tmp/MyMetalExport
            curl -L -o /tmp/MetalToolchain.tgz https://dankulp.com/test/MetalToolChain.tgz
            tar xzf /tmp/MetalToolchain.tgz -C /tmp/MyMetalExport/
            rm -f /tmp/MetalToolchain.tgz
            xcodebuild -importComponent metalToolchain -importPath /tmp/MyMetalExport/MetalToolchain-*.exportedBundle
        fi
    fi
fi

if xcrun -find metal >/dev/null 2>&1; then
    echo "Metal tools have been found and installed."
fi
