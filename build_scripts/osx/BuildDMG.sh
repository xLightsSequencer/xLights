#!/bin/bash

VER=$1
shift

if [ -f ~/.apple-notarize-info ]; then
    #Load the stored signing info
    #This should have something like:
    # export NOTARIZE_EMAIL=dan@kulp.com
    # export NOTARIZE_PWD=...password for the apple notarize system...
    # export DEVELOPMENT_TEAM=U8ZT9RPC6S
    . ~/.apple-notarize-info
fi

rm -f xLights-$VER.dmg
rm -f xLights.dmg

#build the final dmg
hdiutil create -size 192m -fs HFS+ -volname "xLights-$VER" xLights.dmg
hdiutil attach xLights.dmg

for var in "$@"
do
    if [ -e "$var" ]; then
        cp -a "$var" /Volumes/xLights-$VER
    fi
done
ln -s /Applications /Volumes/xLights-$VER/Applications

DEVS=$(hdiutil attach xLights.dmg | cut -f 1)
DEV=$(echo $DEVS | cut -f 1 -d ' ')
 
# Unmount the disk image
hdiutil detach $DEV
 
# Convert the disk image to read-only and compress
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
