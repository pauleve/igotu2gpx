CLEBS *= igotu
TARGET = tester
include(../../clebs.pri)

CONFIG += qtestlib

SOURCES *= \
    igotuconfig.cpp \
    tests.cpp \

HEADERS *= \
    tests.h \