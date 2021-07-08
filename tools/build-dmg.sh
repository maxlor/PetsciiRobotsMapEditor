#!/bin/sh -e

#
# Builds the DMG file for mac. Call with the finished .app file as argument.
# You can set the QTDIR environment variable to override the default.
#

main() {
    local APP=$1
    local QTDIR=${QTDIR:=~/Qt/5.15.2/clang_64}

    if [ ! -d "$APP" ]; then
        echo "Error: application \"$APP\" doesn't exist." >&2
        return 1
    fi

    local VERSION=`grep -A1 CFBundleShortVersionString "$APP/Contents/Info.plist" |  egrep -o '\d[^<]+'`
    local DMG="PetsciiRobotsMapEditor-$VERSION.dmg"

    local DIR=`dirname "$APP"`
    cp "$DIR/tileset.pet" "$APP/Contents/MacOS"
    cp "$DIR/NimbusSansNarrow-Bold.otf" "$APP/Contents/MacOS"
    cp "$DIR/NimbusSansNarrow-Bold.otf.license" "$APP/Contents/MacOS"

    "$QTDIR/bin/macdeployqt" "$APP"

    find "$APP/Contents/PlugIns/imageformats" -type f -not -name "*qsvg.*" -delete

    rm -rf "$DMG"

    hdiutil create -srcfolder "$APP" -nospotlight -noanyowners -noatomic -format UDBZ "$DMG"
}

main "$@"
