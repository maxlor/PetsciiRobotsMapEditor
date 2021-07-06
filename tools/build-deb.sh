#!/bin/sh -e

# Creates a .deb file. Run this with two parameters:
# - the path to PetsciiRobotsMapEditor.pro (it's in the source folder)
# - the path to PetsciiRobotsMapEditor (it's in the build folder once
#   the program has been compiled.)


main() {
	local PROJECT_PATH=$1
	local PROGRAM_PATH=$2
	
	if [ -z "$PROJECT_PATH" ]; then
		printSyntax
		return 1
	elif [ -z "$PROGRAM_PATH" ]; then
		printSyntax
		return 1
	fi
	
	if [ ! -e "$PROJECT_PATH" ]; then
		echo "Error: project file \"$PROJECT_PATH\" not found" >&2
		return 1
	fi
	
	if [ ! -e "$PROGRAM_PATH" ]; then
		echo "Error: program \"$PROGRAM_PATH\" not found >&2"
		return 1
	fi
	
	local PROJECT_DIR=`dirname "$PROJECT_PATH"`
	local PROGRAM=`basename "$PROGRAM_PATH"`
	local BUILD_DIR=`dirname "$PROGRAM_PATH"`
	local DEB_DIR="$BUILD_DIR/deb"
	
	mkdir -p "$DEB_DIR/usr/bin"
	ln -f "$PROGRAM_PATH" "$DEB_DIR/usr/bin/"
	mkdir -p "$DEB_DIR/usr/share/$PROGRAM"
	ln -f "$PROJECT_DIR/res/tileset.pet" "$DEB_DIR/usr/share/$PROGRAM/"
	mkdir -p "$DEB_DIR/usr/share/applications"
	ln -f "$PROJECT_DIR/tools/PetsciiRobotsMapEditor.desktop" "$DEB_DIR/usr/share/applications/"
	mkdir -p "$DEB_DIR/usr/share/icons/hicolor/32x32/apps"
	ln -f "$PROJECT_DIR/res/robot-32.png" "$DEB_DIR/usr/share/icons/hicolor/32x32/apps/PetsciiRobotsMapEditor.png"
	
	export SIZE=`du --apparent-size -ks "$DEB_DIR" | awk '{print $1}'`
	export ARCHITECTURE=`dpkg-architecture -q DEB_TARGET_ARCH`
	export VERSION=`gawk 'match($0, "^\\\\s*APP_VERSION\\\\s*=\\\\s*(.*?)\\\\s*$", a) { print a[1]; }' "$PROJECT_DIR/src/src.pro"`
	
	mkdir -p "$DEB_DIR/DEBIAN"
	# envsubst is in gettext-base package
	envsubst < "$PROJECT_DIR/tools/DEBIAN/control" > "$DEB_DIR/DEBIAN/control"
	
	dpkg-deb -b "$DEB_DIR" "$BUILD_DIR"
}


printSyntax() {
	echo "Syntax: $0 <ProjectFile> <Executable>"
	echo
}

main "$@"
