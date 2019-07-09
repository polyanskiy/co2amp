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
    calc.cpp \
    input.cpp \
    main.cpp \
    memory.cpp \
    opptic_a.cpp \
    optic_af.cpp \
    optic_am.cpp \
    optic_am_amplification.cpp \
    optic_am_band.cpp \
    optic_am_boltzmann.cpp \
    optic_am_dynamics.cpp \
    optic_at.cpp \
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
