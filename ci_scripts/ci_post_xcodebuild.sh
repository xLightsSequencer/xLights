#!/bin/zsh

if [[ -d "$CI_APP_STORE_SIGNED_APP_PATH" ]]; then
    TESTFLIGHT_DIR_PATH=../TestFlight
    mkdir $TESTFLIGHT_DIR_PATH
	git log -1 --pretty=format:"%s" >! $TESTFLIGHT_DIR_PATH/WhatToTest.en-US.txt
fi
