#!/bin/bash

#clean the build area to make sure we have a super clean build
if [ "$1" != "-noclean" ]; then
    xcodebuild -alltargets clean
    rm -rf build
else
    shift
fi

VER=$1

echo "Building $VER"
echo "static const wxString xlights_version_string  = \"${VER}\";" > xLights/xlights_build_version.h
touch xLights/xLightsVersion.h

if [ -f ~/.apple-notarize-info ]; then
    #Load the stored signing info
    #This should have something like:
    # export NOTARIZE_EMAIL=dan@kulp.com
    # export NOTARIZE_PWD=...password for the apple notarize system...
    # export DEVELOPMENT_TEAM=U8ZT9RPC6S
    . ~/.apple-notarize-info
fi

# if using a development team, sign the release build, otherwise, just regular build
if [ "${DEVELOPMENT_TEAM}x" == "x" ]; then
    xcodebuild -alltargets
else
    xcodebuild -target xLights CODE_SIGN_STYLE=Manual DEVELOPMENT_TEAM=${DEVELOPMENT_TEAM} CODE_SIGN_IDENTITY="Developer ID Application"
    if [[ $? != 0 ]]; then
       exit 1
    fi
    xcodebuild -alltargets CODE_SIGN_STYLE=Manual DEVELOPMENT_TEAM=${DEVELOPMENT_TEAM} CODE_SIGN_IDENTITY="Developer ID Application"
    if [[ $? == 0 ]]; then
        ALLTARGETS=1
    else
        ALLTARGETS=0
    fi
fi

cd build/Release

if [ -f xCapture.app/Contents/MacOS/xCapture ]; then
    rm -rf xCapture.app
fi
if [ -f xFade.app/Contents/MacOS/xFade ]; then
    rm -rf xFade.app
fi


if [ "${NOTARIZE_PWD}x" != "x" ]; then
    # if we have a notarize password, we need to package the apps into a dmg
    # and upload to apple for verification and notarizing
    ./BuildDMG.sh $VER xLights.app xSchedule.app xCapture.app xFade.app

    # It's now uploaded, we need to wait until we get the email saying it's been notarized
    # before we continue
    echo "Run xcrun altool --notarization-info UUID -u ${NOTARIZE_EMAIL} -p @env:NOTARIZE_PWD"
    read -p "Press any key to continue... " -n1 -s

    # attache the notarization stamps to the apps
    xcrun stapler staple -v xLights.app
    if [ -f xFade.app/Contents/MacOS/xFade ]; then
        xcrun stapler staple -v xFade.app
    fi
    if [ -f xCapture.app/Contents/MacOS/xCapture ]; then
        xcrun stapler staple -v xCapture.app
    fi

    rm -f xLights.dmg
fi

./BuildDMG.sh $VER xLights.app xCapture.app xFade.app

# cleanup the build version file
cd ../..
rm xLights/xlights_build_version.h
