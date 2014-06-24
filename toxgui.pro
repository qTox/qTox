#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T14:07:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = toxgui
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    core.cpp \
    status.cpp \
    settings.cpp \
    addfriendform.cpp \
    settingsform.cpp \
    editablelabelwidget.cpp \
    copyableelidelabel.cpp \
    elidelabel.cpp \
    esclineedit.cpp \
    friendlist.cpp \
    friend.cpp \
    chatform.cpp \
    chattextedit.cpp \
    friendrequestdialog.cpp \
    friendwidget.cpp \
    groupwidget.cpp \
    group.cpp \
    grouplist.cpp \
    groupchatform.cpp

HEADERS  += widget.h \
    core.h \
    status.h \
    settings.h \
    addfriendform.h \
    settingsform.h \
    editablelabelwidget.h \
    elidelabel.hpp \
    copyableelidelabel.h \
    elidelabel.h \
    esclineedit.h \
    friendlist.h \
    friend.h \
    chatform.h \
    chattextedit.h \
    friendrequestdialog.h \
    friendwidget.h \
    groupwidget.h \
    group.h \
    grouplist.h \
    groupchatform.h

FORMS    += widget.ui

CONFIG += c++11

RESOURCES += \
    res.qrc

LIBS += -ltoxcore -lsodium
