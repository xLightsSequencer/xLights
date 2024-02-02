#!/bin/zsh

if [[ -d "$CI_APP_STORE_SIGNED_APP_PATH" ]]; then
    TESTFLIGHT_DIR_PATH=../TestFlight
    mkdir $TESTFLIGHT_DIR_PATH
    git status | head -n 1 | tee $TESTFLIGHT_DIR_PATH/WhatToTest.en-US.txt
    git log --since="48 hours ago" --pretty=format:"%s" | tee -a $TESTFLIGHT_DIR_PATH/WhatToTest.en-US.txt
fi
