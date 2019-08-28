# -------------------------------------------------
# Project created by QtCreator 2008-12-24T10:02:38
# -------------------------------------------------
TARGET = co2amp
TEMPLATE = app
SOURCES += main.cpp \
    calc.cpp \
    config.cpp \
    data.cpp \
    environment.cpp \
    mainwindow.cpp \
    clipboard.cpp \
    project.cpp \
    tab0-config.cpp \
    tab1-process.cpp \
    tab2-plot.cpp \
    tab3-comments.cpp \
    tab4-about.cpp \
    update.cpp \
    yaml.cpp
HEADERS += \
    co2amp.h
OTHER_FILES += icon.rc
FORMS += mainwindow.ui
RESOURCES += images.qrc
RC_FILE = icon.rc
CONFIG += qt
QT += widgets
QT += svg
