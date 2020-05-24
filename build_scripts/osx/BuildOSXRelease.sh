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
    xcodebuild -alltargets -jobs=10
else
    xcodebuild -target xLights -jobs=10 CODE_SIGN_STYLE=Manual DEVELOPMENT_TEAM=${DEVELOPMENT_TEAM} CODE_SIGN_IDENTITY="Developer ID Application"
    if [[ $? != 0 ]]; then
       exit 1
    fi
    xcodebuild -alltargets -jobs=10 CODE_SIGN_STYLE=Manual DEVELOPMENT_TEAM=${DEVELOPMENT_TEAM} CODE_SIGN_IDENTITY="Developer ID Application"
    if [[ $? == 0 ]]; then
        ALLTARGETS=1
    else
        ALLTARGETS=0
    fi
fi

cd build/Release

if [ "${NOTARIZE_PWD}x" != "x" ]; then
    # if we have a notarize password, we need to package the apps into a dmg
    # and upload to apple for verification and notarizing
    rm -f xLights-$VER.dmg
    rm -f xLights.dmg
    hdiutil create -size 192m -fs HFS+ -volname "xLights-$VER" xLights.dmg
    hdiutil attach xLights.dmg

    cp -a xLights.app /Volumes/xLights-$VER
    if [ -f xSchedule.app/Contents/MacOS/xSchedule ]; then
        cp -a xSchedule.app /Volumes/xLights-$VER
    fi
    if [ -f xCapture.app/Contents/MacOS/xCapture ]; then
        cp -a xCapture.app /Volumes/xLights-$VER
    fi
    if [ -f xFade.app/Contents/MacOS/xFade ]; then
        cp -a xFade.app /Volumes/xLights-$VER
    fi
    ln -s /Applications /Volumes/xLights-$VER/Applications

    DEVS=$(hdiutil attach xLights.dmg | cut -f 1)
    DEV=$(echo $DEVS | cut -f 1 -d ' ')

    # Unmount the disk image
    hdiutil detach $DEV

    # Convert the disk image to read-only
    hdiutil convert xLights.dmg -format UDZO -o xLights-$VER.dmg

#    codesign --force --sign "Developer ID Application: Daniel Kulp" xLights-$VER.dmg
#    spctl -a -t open --context context:primary-signature -v xLights-$VER.dmg

    xcrun altool --notarize-app -f xLights-$VER.dmg  --primary-bundle-id org.xlights -u ${NOTARIZE_EMAIL} -p @env:NOTARIZE_PWD

    # It's now uploaded, we need to wait until we get the email saying it's been notarized
    # before we continue
    echo "Run xcrun altool --notarization-info UUID -u ${NOTARIZE_EMAIL} -p @env:NOTARIZE_PWD"
    read -p "Press any key to continue... " -n1 -s

    # attache the notarization stamps to the apps
    xcrun stapler staple -v xLights.app
    if [ -f xSchedule.app/Contents/MacOS/xSchedule ]; then
        xcrun stapler staple -v xSchedule.app
    fi
    if [ -f xFade.app/Contents/MacOS/xFade ]; then
        xcrun stapler staple -v xFade.app
    fi
    if [ -f xCapture.app/Contents/MacOS/xCapture ]; then
        xcrun stapler staple -v xCapture.app
    fi

    rm -f xLights.dmg
fi

#build the tar.gz
# rm -f xLights-MAC-$VER.tar.gz
# tar -czf  xLights-MAC-$VER.tar.gz xLights.app xSchedule.app xCapture.app xFade.app

rm -f xLights-$VER.dmg
rm -f xLights.dmg

#build the final dmg
hdiutil create -size 192m -fs HFS+ -volname "xLights-$VER" xLights.dmg
hdiutil attach xLights.dmg

cp -a xLights.app /Volumes/xLights-$VER
if [ -f xSchedule.app/Contents/MacOS/xSchedule ]; then
    cp -a xSchedule.app /Volumes/xLights-$VER
fi
if [ -f xCapture.app/Contents/MacOS/xCapture ]; then
    cp -a xCapture.app /Volumes/xLights-$VER
fi
if [ -f xFade.app/Contents/MacOS/xFade ]; then
    cp -a xFade.app /Volumes/xLights-$VER
fi
ln -s /Applications /Volumes/xLights-$VER/Applications

DEVS=$(hdiutil attach xLights.dmg | cut -f 1)
DEV=$(echo $DEVS | cut -f 1 -d ' ')
 
# Unmount the disk image
hdiutil detach $DEV
 
# Convert the disk image to read-only
hdiutil convert xLights.dmg -format UDZO -o xLights-$VER.dmg

# Sign the DMG
codesign --force --sign "Developer ID Application: Daniel Kulp" xLights-$VER.dmg
spctl -a -t open --context context:primary-signature -v xLights-$VER.dmg

if [ "${NOTARIZE_PWD}x" != "x" ]; then
    # Now send the final DMG off to apple to notarize.  This DMG has the notarized .app's
    # so it's different than the previous DMG

    xcrun altool --notarize-app -f xLights-$VER.dmg  --primary-bundle-id org.xlights -u ${NOTARIZE_EMAIL} -p @env:NOTARIZE_PWD
    echo "Run xcrun altool --notarization-info UUID -u ${NOTARIZE_EMAIL} -p @env:NOTARIZE_PWD"
    read -p "Press any key to continue... " -n1 -s

    # staple the DMG's notarization to the dmg
    xcrun stapler staple -v xLights-$VER.dmg
fi

# cleanup the build version file
cd ../..
rm xLights/xlights_build_version.h
