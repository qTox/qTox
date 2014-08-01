#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T14:07:35
#
#-------------------------------------------------


#    Copyright (C) 2014 by Project Tox <https://tox.im>
#
#    This file is part of qTox, a Qt-based graphical interface for Tox.
#
#    This program is libre software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#    See the COPYING file for more details.


QT       += core gui network multimedia multimediawidgets xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET    = qtox
TEMPLATE  = app
FORMS    += widget.ui
CONFIG   += c++11

TRANSLATIONS = translations/de.ts \
               translations/fr.ts \
               translations/it.ts \
               translations/ru.ts

RESOURCES += res.qrc

target.path = /usr/local/bin
INSTALLS += target

INCLUDEPATH += libs/include
win32 {
    LIBS += $$PWD/libs/lib/libtoxav.a $$PWD/libs/lib/libopus.a $$PWD/libs/lib/libvpx.a $$PWD/libs/lib/libtoxcore.a -lws2_32 $$PWD/libs/lib/libsodium.a -lpthread -liphlpapi
} else {
    LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -lsodium -lvpx
}

#### Static linux build
#LIBS += -Wl,-Bstatic -ltoxcore -ltoxav -lsodium -lvpx -lopus \
#      -lgstbase-0.10 -lgstreamer-0.10 -lgmodule-2.0 -lgstaudio-0.10 -lxml2 \
#       -lX11-xcb -lXi -lxcb-render-util -lxcb-glx -lxcb-render -ldbus-1 \
#    -lxcb -lXau -lXdmcp -lxcb-image -lxcb-icccm -lxcb-sync -lxcb-xfixes -lxcb-shm -lxcb-randr -lxcb-shape \
#    -lxcb-keysyms -lxcb-xkb -lfontconfig -lfreetype -lXrender -lXext -lX11 \
#   -lpng -lz -licui18n -licuuc -licudata -lm -lgthread-2.0 \
#     -pthread -lrt -lGL -lpthread -Wl,-Bdynamic -ldl -lc
#QMAKE_CXXFLAGS += -Os -flto -static-libstdc++ -static-libgcc

HEADERS  += widget/form/addfriendform.h \
    widget/form/chatform.h \
    widget/form/groupchatform.h \
    widget/form/settingsform.h \
    widget/form/filesform.h \
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
    core.h \
    friendlist.h \
    cdata.h \
    cstring.h \
    audiobuffer.h \
    widget/selfcamview.h \
    widget/videosurface.h \
    widget/camera.h \
    widget/netcamview.h \
    widget/tool/clickablelabel.h \
    smileypack.h \
    widget/emoticonswidget.h

SOURCES += \
    widget/form/addfriendform.cpp \
    widget/form/chatform.cpp \
    widget/form/groupchatform.cpp \
    widget/form/settingsform.cpp \
    widget/form/filesform.cpp \
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
    cdata.cpp \
    cstring.cpp \
    audiobuffer.cpp \
    widget/selfcamview.cpp \
    widget/videosurface.cpp \
    widget/camera.cpp \
    widget/netcamview.cpp \
    widget/tool/clickablelabel.cpp \
    smileypack.cpp \
    widget/emoticonswidget.cpp
