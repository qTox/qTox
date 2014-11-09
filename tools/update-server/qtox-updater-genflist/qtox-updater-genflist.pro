TEMPLATE = app
CONFIG += console c++11
QT += core
LIBS += -lsodium

SOURCES += main.cpp \
    serialize.cpp

HEADERS += \
    serialize.h
