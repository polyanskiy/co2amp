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
    misc.cpp \
    optic_A.cpp \
    optic_A_amplification.cpp \
    optic_A_band.cpp \
    optic_A_boltzmann.cpp \
    optic_A_dynamics.cpp \
    optic_C.cpp \
    optic_F.cpp \
    optic_L.cpp \
    optic_M.cpp \
    optic_P.cpp \
    optic_S.cpp \
    output.cpp \
    pulse.cpp
HEADERS += \
    co2amp.h
#QMAKE_CFLAGS += -ffast-math -fopenmp
QMAKE_CXXFLAGS += -ffast-math -fopenmp -Wno-sign-compare
QMAKE_LFLAGS += -fopenmp


#LIBS += -L$$PWD/../libyaml/ -lyaml
#LIBS += -L$$PWD/../libyaml/ -llibyaml.dll
