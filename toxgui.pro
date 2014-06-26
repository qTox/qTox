#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T14:07:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = toxgui
TEMPLATE = app

HEADERS  += widget/form/addfriendform.h \
    widget/form/chatform.h \
    widget/form/groupchatform.h \
    widget/form/settingsform.h \
    widget/tool/chattextedit.h \
    widget/tool/copyableelidelabel.h \
    widget/tool/editablelabelwidget.h \
    widget/tool/elidelabel.h \
    widget/tool/esclineedit.h \
    widget/tool/friendrequestdialog.h \
    widget/filetransfertwidget.h \
    widget/friendwidget.h \
    widget/groupwidget.h \
    widget/widget.h \
    friend.h \
    group.h \
    grouplist.h \
    settings.h \
    status.h \
    core.h \
    friendlist.h \
    cdata.h \
    cstring.h

FORMS    += widget.ui

CONFIG += c++11

RESOURCES += \
    res.qrc

LIBS += -ltoxcore -ltoxav -lsodium

SOURCES += \
    widget/form/addfriendform.cpp \
    widget/form/chatform.cpp \
    widget/form/groupchatform.cpp \
    widget/form/settingsform.cpp \
    widget/tool/chattextedit.cpp \
    widget/tool/copyableelidelabel.cpp \
    widget/tool/editablelabelwidget.cpp \
    widget/tool/elidelabel.cpp \
    widget/tool/esclineedit.cpp \
    widget/tool/friendrequestdialog.cpp \
    widget/filetransfertwidget.cpp \
    widget/friendwidget.cpp \
    widget/groupwidget.cpp \
    widget/widget.cpp \
    core.cpp \
    friend.cpp \
    friendlist.cpp \
    group.cpp \
    grouplist.cpp \
    main.cpp \
    settings.cpp \
    status.cpp \
    cdata.cpp \
    cstring.cpp
