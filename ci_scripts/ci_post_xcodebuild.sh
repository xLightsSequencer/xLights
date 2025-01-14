#!/bin/zsh

# By default, xcodecloud does --depth 1 do NO history is available.  We
# need to fetch a longer history and fetch the tags so
# we can get the list of changes.   200 commits should be enough
# considering how often we release.
git fetch --tags -f --deepen 200 origin ${CI_COMMIT}
git status | head -n 1 | tee WhatToTest.en-US.txt
git log $(git describe --abbrev=0 HEAD^)..HEAD --format="%s (%aN)" | tee -a WhatToTest.en-US.txt


if [[ -d "$CI_APP_STORE_SIGNED_APP_PATH" ]]; then
    TESTFLIGHT_DIR_PATH=../TestFlight
    mkdir $TESTFLIGHT_DIR_PATH
    cp -f WhatToTest.en-US.txt $TESTFLIGHT_DIR_PATH
fi
