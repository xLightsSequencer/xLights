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


if [[ -d "$CI_APP_STORE_SIGNED_APP_PATH" ]]; then
    TESTFLIGHT_DIR_PATH=../TestFlight
    mkdir $TESTFLIGHT_DIR_PATH
    cp -f WhatToTest.en-US.txt $TESTFLIGHT_DIR_PATH
fi

