#!/bin/sh

#
# Creates robot.icns, the icon file used on Mac OS X.
#

ICONUTIL=`which iconutil`

if [ -z "$ICONUTIL" ]; then
	echo "Error: iconutil not found" >&2
	exit 1
fi

if [ ! -e robot-16.png ]; then
	echo "Error: robot-16.png not found" >&2
	exit 1
fi

if [ ! -e robot-32.png ]; then
	echo "Error: robot-32.png not found" >&2
	exit 1
fi

mkdir robot.iconset
ln robot-16.png robot.iconset/icon_16x16.png
ln robot-32.png robot.iconset/icon_16x16@2x.png
ln robot-32.png robot.iconset/icon_32x32.png
iconutil --convert icns robot.iconset
rm -rf robot.iconset
