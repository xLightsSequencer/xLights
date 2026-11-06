#!/bin/zsh

# Report ccache hit rate so we can see caching effectiveness in the log.
if command -v ccache >/dev/null 2>&1; then
    ccache --show-stats -v || true
fi

# By default, xcodecloud does --depth 1 so NO history is available.  We
# need to fetch a longer history and fetch the tags so
# we can get the list of changes.   400 commits should be enough
# considering how often we release.
git fetch --tags -f --deepen 400 origin ${CI_COMMIT}
git submodule update --init 
git status | head -n 1 | tee WhatToTest.en-US.txt
git log $(git describe --abbrev=0 HEAD^)..HEAD --format="%s (%aN)" | head -n 50 | tee -a WhatToTest.en-US.txt


# Nightly: run the unit tests and the render regression on this builder, push the results to GitHub, and
# add a one-line summary to the TestFlight notes. Opt in with XL_NIGHTLY_TESTS=1 in the workflow. Best
# effort: whatever happens here must not fail the build or hold up the TestFlight upload.
if [[ "$XL_NIGHTLY_TESTS" == "1" && "$CI_XCODEBUILD_ACTION" == "archive" && "$CI_PRODUCT_PLATFORM" == "macOS" && "${CI_XCODEBUILD_EXIT_CODE:-0}" == "0" ]]; then
    ( cd "${CI_PRIMARY_REPOSITORY_PATH:-..}" && ./ci_scripts/xcode_cloud_tests.sh /tmp/xl_test_summary.txt ) || echo "xcode_cloud_tests.sh failed (ignored)"
    [[ -s /tmp/xl_test_summary.txt ]] && { echo "" ; cat /tmp/xl_test_summary.txt; } >> WhatToTest.en-US.txt
fi

if [[ -d "$CI_APP_STORE_SIGNED_APP_PATH" ]]; then
    TESTFLIGHT_DIR_PATH=../TestFlight
    mkdir $TESTFLIGHT_DIR_PATH
    cp -f WhatToTest.en-US.txt $TESTFLIGHT_DIR_PATH
fi

