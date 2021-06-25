QT       += core gui svg widgets
CONFIG += c++17
APP_VERSION = 0.1

DEFINES += APP_VERSION=$${APP_VERSION}

SOURCES += \
    abstracttilewidget.cpp \
    constants.cpp \
    coordinatewidget.cpp \
    iconfactory.cpp \
    main.cpp \
    mainwindow.cpp \
    map.cpp \
    mapwidget.cpp \
    multisignalblocker.cpp \
    objecteditwidget.cpp \
    scrollarea.cpp \
    tile.cpp \
    tileset.cpp \
    tilewidget.cpp

HEADERS += \
    abstracttilewidget.h \
    constants.h \
    coordinatewidget.h \
    iconfactory.h \
    mainwindow.h \
    map.h \
    mapwidget.h \
    multisignalblocker.h \
    objecteditwidget.h \
    scrollarea.h \
    tile.h \
    tileset.h \
    tilewidget.h

FORMS += \
    mainwindow.ui \
    objecteditwidget.ui
    
RESOURCES += \
    res.qrc

OTHER_FILES += \
    res/NimbusSansNarrow-Bold.otf \
    res/NimbusSansNarrow-Bold.otf.license

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
