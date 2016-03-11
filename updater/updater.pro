#-------------------------------------------------
#
# Project created by QtCreator 2014-11-09T21:09:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtox-updater
TEMPLATE = app

CONFIG += c++11

QMAKE_CXXFLAGS += -fno-exceptions

SOURCES += main.cpp\
        widget.cpp \
    update.cpp \
    serialize.cpp \
    settings.cpp

HEADERS  += widget.h \
    update.h \
    serialize.h \
    settings.h

FORMS    += widget.ui

RESOURCES += \
    res.qrc

INCLUDEPATH += libs/include

RC_FILE = windows/updater.rc

LIBS += -L$$PWD/libs/lib/ -lsodium

win32 {
    LIBS += -lshell32 -luuid
}
