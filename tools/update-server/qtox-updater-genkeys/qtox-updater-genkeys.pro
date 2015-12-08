QT += core
QT -= gui

TARGET = qtox-updater-genkeys
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp


LIBS += -lsodium
