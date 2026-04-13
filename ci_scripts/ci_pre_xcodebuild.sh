#!/bin/sh

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

# ccache for compiler caching against the remote backend.
# CCACHE_REMOTE_URL is provided as a secret environment variable in the
# Xcode Cloud workflow. When unset (e.g. on a branch that doesn't have the
# secret), ccache falls back to its default behavior.
brew install ccache
if [ -n "$CCACHE_REMOTE_URL" ]; then
    mkdir -p "$HOME/.config/ccache"
    cat > "$HOME/.config/ccache/ccache.conf" <<EOF
remote_storage = $CCACHE_REMOTE_URL
remote_only    = true
sloppiness     = pch_defines,time_macros,include_file_mtime,include_file_ctime,locale,modules
EOF
    ccache --zero-stats
    echo "ccache configured with remote storage"
else
    echo "CCACHE_REMOTE_URL not set; skipping ccache remote config"
fi

xcrun -find metal
xcodebuild -downloadComponent MetalToolchain
xcrun -find metal
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
