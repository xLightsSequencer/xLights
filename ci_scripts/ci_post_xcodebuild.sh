#!/bin/zsh

if [[ -d "$CI_APP_STORE_SIGNED_APP_PATH" ]]; then
    TESTFLIGHT_DIR_PATH=../TestFlight
    mkdir $TESTFLIGHT_DIR_PATH
    git log --since="28 hours ago" --pretty=format:"%s" | tee $TESTFLIGHT_DIR_PATH/WhatToTest.en-US.txt
    # git status | head -n 1 >> $TESTFLIGHT_DIR_PATH/WhatToTest.en-US.txt
fi
