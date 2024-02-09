#!/bin/zsh

if [[ -d "$CI_APP_STORE_SIGNED_APP_PATH" ]]; then
    TESTFLIGHT_DIR_PATH=../TestFlight
    mkdir $TESTFLIGHT_DIR_PATH
    git status | head -n 1 | tee $TESTFLIGHT_DIR_PATH/WhatToTest.en-US.txt
    git log $(git describe --tags --abbrev=0 HEAD^)..HEAD --oneline | colrm 1 10 | tee -a $TESTFLIGHT_DIR_PATH/WhatToTest.en-US.txt
fi
