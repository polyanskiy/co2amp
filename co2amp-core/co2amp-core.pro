# -------------------------------------------------
# Project created by QtCreator 2008-12-24T10:01:05
# -------------------------------------------------
QT -= core \
    gui
TARGET = co2amp-core
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += output.c \
    optics.c \
    memory.c \
    input.c \
    dynamics.c \
    band.c \
    boltzmann.c \
    amplification.c \
    main.c \
    calc.c \
    yaml.c
HEADERS += \
    co2amp.h
QMAKE_CFLAGS += -ffast-math -fopenmp
QMAKE_LFLAGS += -fopenmp


LIBS += -L$$PWD/../libyaml/ -lyaml
#LIBS += -L$$PWD/../libyaml/ -llibyaml.dll
