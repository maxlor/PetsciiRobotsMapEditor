TEMPLATE = app
QT       += core gui svg widgets
CONFIG += c++17
CONFIG -= debug_and_release_target
GITREV = $$system(git --git-dir=$$PWD/../.git describe --always --abbrev=0)
GITCOUNT = $$system(git --git-dir=$$PWD/../.git rev-list HEAD --count)
APP_VERSION = 1.2.0
VERSION = $${APP_VERSION}.$${GITCOUNT}
macx {
    TARGET = "PETSCII Robots Map Editor"
} else {
    TARGET = PetsciiRobotsMapEditor
}
QMAKE_TARGET_PRODUCT = PETSCII Robots Map Editor
QMAKE_TARGET_COPYRIGHT = Copyright 2021 Benjamin Lutz
RC_ICONS = ../res/robot.ico
ICON = ../res/robot.icns
QMAKE_INFO_PLIST = ../res/Info.plist
QMAKE_TARGET_BUNDLE_PREFIX = com.maxlor

DEFINES += APP_VERSION=$${APP_VERSION} GITREV=$${GITREV}

SOURCES += \
    abstracttilewidget.cpp \
    constants.cpp \
    coordinatewidget.cpp \
    iconfactory.cpp \
    main.cpp \
    mainwindow.cpp \
    map.cpp \
    mapcheck.cpp \
    mapcommands.cpp \
    mapcontroller.cpp \
    mapobject.cpp \
    mapwidget.cpp \
    multisignalblocker.cpp \
    objecteditwidget.cpp \
    scrollarea.cpp \
    tile.cpp \
    tileset.cpp \
    tilewidget.cpp \
    util.cpp \
    validationdialog.cpp

HEADERS += \
    abstracttilewidget.h \
    constants.h \
    coordinatewidget.h \
    iconfactory.h \
    mainwindow.h \
    map.h \
    mapcheck.h \
    mapcommands.h \
    mapcontroller.h \
    mapobject.h \
    mapwidget.h \
    multisignalblocker.h \
    objecteditwidget.h \
    scrollarea.h \
    tile.h \
    tileset.h \
    tilewidget.h \
    util.h \
    validationdialog.h

FORMS += \
    mainwindow.ui \
    objecteditwidget.ui \
    validationdialog.ui
    
RESOURCES += \
    ../res/res.qrc

OTHER_FILES += \
    ../res/NimbusSansNarrow-Bold.otf \
    ../res/NimbusSansNarrow-Bold.otf.license \
    ../res/tileset.c64 \
    ../res/tileset.pet

win32:contains(QMAKE_CXX, cl) {
    # permissive-: enables alternative operator names
	# disable C4715: cot all control paths return a value. This is triggered when going through
	#    a switch() of an enum class objects, even when all possible enum class values are
	#    handled. Clang on Linux does this properly, so rely on that in this case.
	# disable C4267: conversion from size_t to uint8_t, possible loss of data.
	QMAKE_CXXFLAGS += -permissive- -wd4715 -wd4267
}

# copies the given files to the destination directory
defineTest(copyToDestDir) {
    files = $$1
    dir = $$2
    win32:dir ~= s,/,\\,g

    for(file, files) {
        file = $$PWD/$$file
        win32:file ~= s,/,\\,g

        QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_quote($$file) $$shell_quote($$dir) $$escape_expand(\\n\\t)
    }

    export(QMAKE_POST_LINK)
}

copyToDestDir($$OTHER_FILES, $$OUT_PWD/)
