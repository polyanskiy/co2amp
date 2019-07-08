# -------------------------------------------------
# Project created by QtCreator 2008-12-24T10:01:05
# -------------------------------------------------
QT -= core \
    gui
TARGET = co2amp-core
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += \
    am_amplification.cpp \
    am_band.cpp \
    am_boltzmann.cpp \
    am_dynamics.cpp \
    calc.cpp \
    component_am.cpp \
    input.cpp \
    main.cpp \
    memory.cpp \
    optics.cpp \
    output.cpp \
    yaml.cpp
HEADERS += \
    co2amp.h
QMAKE_CFLAGS += -ffast-math -fopenmp
QMAKE_CXXFLAGS += -ffast-math -fopenmp
QMAKE_LFLAGS += -fopenmp


LIBS += -L$$PWD/../libyaml/ -lyaml
#LIBS += -L$$PWD/../libyaml/ -llibyaml.dll
