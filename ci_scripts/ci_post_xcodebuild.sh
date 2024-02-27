#!/bin/zsh

git fetch --tags -f
git status | head -n 1 | tee WhatToTest.en-US.txt
git log $(git describe --abbrev=0 HEAD^)..HEAD --oneline | colrm 1 10 | tee -a WhatToTest.en-US.txt

if [[ -d "$CI_APP_STORE_SIGNED_APP_PATH" ]]; then
    TESTFLIGHT_DIR_PATH=../TestFlight
    mkdir $TESTFLIGHT_DIR_PATH
    cp -f WhatToTest.en-US.txt $TESTFLIGHT_DIR_PATH
fi
