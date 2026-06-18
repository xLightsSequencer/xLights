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

#install zstd so we can decompress the deps. Retry: Homebrew's bottle
#downloads occasionally fail with transient DNS errors on the Xcode Cloud
#builders, and without zstd on PATH the download_deps tar extract silently
#falls over with "unable to run program zstd -d -qq".
for i in 1 2 3; do
    brew install zstd && break
    echo "brew install zstd failed (attempt $i); retrying after 15s..."
    sleep 15
done
if ! command -v zstd >/dev/null 2>&1; then
    echo "ci_pre_xcodebuild: zstd is not on PATH after 3 brew install attempts" >&2
    exit 1
fi

# Fetch the prebuilt dependency tarball (XCFrameworks, libs, headers)
# into macOS/dependencies/. This MUST run before xcodebuild begins:
# Xcode validates XCFramework references during CreateBuildDescription,
# which happens before any target's build phases execute, so a cold-slate
# CI VM would fail with "There is no XCFramework found at ..." before the
# xLights-Apple-core build phase that normally handles the download could
# run. Local developers still get the auto-download via that build phase;
# the script is idempotent so calling it from both places is safe.
# CI_DERIVED_DATA_PATH on Xcode Cloud points at the DerivedData dir, not
# the macOS/ source dir, so cd to the source macOS/ before invoking.
(cd $CI_PRIMARY_REPOSITORY_PATH/macOS && scripts/download_deps)

# ccache for compiler caching against the remote backend.
# CCACHE_REMOTE_URL is provided as a secret environment variable in the
# Xcode Cloud workflow. When unset (e.g. on a branch that doesn't have the
# secret), ccache falls back to its default behavior.
brew install ccache
if [ -n "$CCACHE_REMOTE_URL" ]; then
    ccache --set-config remote_storage="$CCACHE_REMOTE_URL"
    ccache --set-config remote_only=true
    ccache --set-config sloppiness=pch_defines,time_macros,include_file_mtime,include_file_ctime,locale,modules
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
