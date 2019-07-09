# -------------------------------------------------
# Project created by QtCreator 2008-12-24T10:02:38
# -------------------------------------------------
TARGET = co2amp
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    optics.cpp \
    plot.cpp \
    clipboard.cpp \
    settings.cpp \
    misc.cpp \
    update.cpp
HEADERS += mainwindow.h
OTHER_FILES += icon.rc
FORMS += mainwindow.ui
RESOURCES += images.qrc
RC_FILE = icon.rc
CONFIG += qt
QT += widgets
QT += svg
