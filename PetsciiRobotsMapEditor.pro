QT       += core gui widgets
CONFIG += c++17
APP_VERSION = 0.1

DEFINES += APP_VERSION=$${APP_VERSION}

SOURCES += \
    abstracttilewidget.cpp \
    constants.cpp \
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
