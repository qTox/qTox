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


QT       += core gui network xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET    = qtox
TEMPLATE  = app
FORMS    += \
    mainwindow.ui
CONFIG   += c++11

TRANSLATIONS = translations/de.ts \
               translations/fr.ts \
               translations/it.ts \
               translations/ru.ts \
               translations/pl.ts \
               translations/fi.ts \
               translations/mannol.ts

RESOURCES += res.qrc

contains(JENKINS,YES) {
	INCLUDEPATH += ./libs/include/
} else {
	INCLUDEPATH += libs/include
}

# Rules for Windows, Mac OSX, and Linux
win32 {
    LIBS += -L$$PWD/libs/lib -lopencv_core249 -lopencv_highgui249 -lopencv_imgproc249 -lOpenAL32
    LIBS += $$PWD/libs/lib/libtoxav.a $$PWD/libs/lib/libopus.a $$PWD/libs/lib/libvpx.a $$PWD/libs/lib/libtoxcore.a -lws2_32 $$PWD/libs/lib/libsodium.a -lpthread -liphlpapi
} else {
    macx {
        LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -lsodium -lvpx -framework OpenAL -lopencv_core -lopencv_highgui
    } else {
        # If we're building a package, static link libtox[core,av] and libsodium, since they are not provided by any package
        contains(STATICPKG, YES) {
            target.path = /usr/bin
            INSTALLS += target
            LIBS += -L$$PWD/libs/lib/ -Wl,-Bstatic -ltoxcore -ltoxav -lsodium -Wl,-Bdynamic -lopus -lvpx -lopenal -lopencv_core -lopencv_highgui
        } else {
            LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -lvpx -lopenal -lopencv_core -lopencv_highgui
        }

        contains(JENKINS, YES) {
            LIBS = ./libs/lib/libtoxav.a ./libs/lib/libvpx.a ./libs/lib/libopus.a ./libs/lib/libtoxcore.a ./libs/lib/libsodium.a -lopencv_core -lopencv_highgui -lopenal
        }
    }
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
    widget/form/filesform.h \
    widget/tool/chattextedit.h \
    widget/tool/friendrequestdialog.h \
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
    widget/selfcamview.h \
    widget/camera.h \
    widget/netcamview.h \
    smileypack.h \
    widget/emoticonswidget.h \
    style.h \
    widget/adjustingscrollarea.h \
    widget/croppinglabel.h \
    widget/friendlistwidget.h \
    widget/genericchatroomwidget.h \
    widget/form/genericchatform.h \
    widget/tool/chatactions/chataction.h \
    widget/chatareawidget.h \
    filetransferinstance.h \
    corestructs.h \
    coredefines.h \
    coreav.h \
    widget/settingsdialog.h \
    widget/tool/chatactions/messageaction.h \
    widget/tool/chatactions/filetransferaction.h \
    widget/tool/chatactions/systemmessageaction.h \
    widget/maskablepixmapwidget.h

SOURCES += \
    widget/form/addfriendform.cpp \
    widget/form/chatform.cpp \
    widget/form/groupchatform.cpp \
    widget/form/filesform.cpp \
    widget/tool/chattextedit.cpp \
    widget/tool/friendrequestdialog.cpp \
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
    widget/selfcamview.cpp \
    widget/camera.cpp \
    widget/netcamview.cpp \
    smileypack.cpp \
    widget/emoticonswidget.cpp \
    style.cpp \
    widget/adjustingscrollarea.cpp \
    widget/croppinglabel.cpp \
    widget/friendlistwidget.cpp \
    coreav.cpp \
    widget/genericchatroomwidget.cpp \
    widget/form/genericchatform.cpp \
    widget/tool/chatactions/chataction.cpp \
    widget/chatareawidget.cpp \
    filetransferinstance.cpp \
    corestructs.cpp \
    widget/settingsdialog.cpp \
    widget/tool/chatactions/messageaction.cpp \
    widget/tool/chatactions/filetransferaction.cpp \
    widget/tool/chatactions/systemmessageaction.cpp \
    widget/maskablepixmapwidget.cpp
