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


QT       += core gui network xml opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET    = qtox
TEMPLATE  = app
FORMS    += \
    mainwindow.ui \
    widget/form/settings/generalsettings.ui \
    widget/form/settings/avsettings.ui \
    widget/form/settings/identitysettings.ui
CONFIG   += c++11

TRANSLATIONS = translations/de.ts \
               translations/fr.ts \
               translations/it.ts \
               translations/ru.ts \
               translations/pirate.ts \
               translations/pl.ts \
               translations/fi.ts \
               translations/mannol.ts \
               translations/uk.ts

RESOURCES += res.qrc

contains(JENKINS,YES) {
	INCLUDEPATH += ./libs/include/
} else {
	INCLUDEPATH += libs/include
}

# Rules for Windows, Mac OSX, and Linux
win32 {
    LIBS += -liphlpapi -L$$PWD/libs/lib -ltoxav -ltoxcore -lvpx -lpthread
    LIBS += -L$$PWD/libs/lib -lopencv_core248 -lopencv_highgui248 -lopencv_imgproc248 -lOpenAL32 -lopus
    LIBS += -lz -lopengl32 -lole32 -loleaut32 -luuid -lvfw32 -ljpeg -ltiff -lpng -ljasper -lIlmImf -lHalf -lws2_32
} else {
    macx {
        LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -lsodium -lvpx -framework OpenAL -lopencv_core -lopencv_highgui
    } else {
        # If we're building a package, static link libtox[core,av] and libsodium, since they are not provided by any package
        contains(STATICPKG, YES) {
            target.path = /usr/bin
            INSTALLS += target
            LIBS += -L$$PWD/libs/lib/ -lopus -lvpx -lopenal -Wl,-Bstatic -ltoxcore -ltoxav -lsodium -lopencv_highgui -lopencv_imgproc -lopencv_core -lz -Wl,-Bdynamic
	    LIBS += -Wl,-Bstatic -ljpeg -ltiff -lpng -ljasper -lIlmImf -lIlmThread -lIex -ldc1394 -lraw1394 -lHalf -lz -llzma -ljbig
	    LIBS += -Wl,-Bdynamic -ltbb -lv4l1 -lv4l2 -lgnutls -lrtmp -lgnutls -lavformat -lavcodec -lavutil -lavfilter -lswscale -lusb-1.0

        } else {
            LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -lvpx -lopenal -lopencv_core -lopencv_highgui -lopencv_imgproc
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
    widget/form/settingswidget.h \
    widget/form/settings/genericsettings.h \
    widget/form/settings/generalform.h \
    widget/form/settings/identityform.h \
    widget/form/settings/privacyform.h \
    widget/form/settings/avform.h \
    widget/form/filesform.h \
    widget/tool/chattextedit.h \
    widget/tool/friendrequestdialog.h \
    widget/friendwidget.h \
    widget/groupwidget.h \
    widget/widget.h \
    friend.h \
    group.h \
    grouplist.h \
    misc/settings.h \
    core.h \
    friendlist.h \
    misc/cdata.h \
    misc/cstring.h \
    widget/camera.h \
    widget/netcamview.h \
    misc/smileypack.h \
    widget/emoticonswidget.h \
    misc/style.h \
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
    widget/tool/chatactions/messageaction.h \
    widget/tool/chatactions/filetransferaction.h \
    widget/tool/chatactions/systemmessageaction.h \
    widget/tool/chatactions/actionaction.h \
    widget/maskablepixmapwidget.h \
    videosource.h \
    cameraworker.h \
    widget/videosurface.h

SOURCES += \
    widget/form/addfriendform.cpp \
    widget/form/chatform.cpp \
    widget/form/groupchatform.cpp \
    widget/form/settingswidget.cpp \
    widget/form/settings/generalform.cpp \
    widget/form/settings/identityform.cpp \
    widget/form/settings/privacyform.cpp \
    widget/form/settings/avform.cpp \
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
    misc/settings.cpp \
    misc/cdata.cpp \
    misc/cstring.cpp \
    widget/camera.cpp \
    widget/netcamview.cpp \
    misc/smileypack.cpp \
    widget/emoticonswidget.cpp \
    misc/style.cpp \
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
    widget/tool/chatactions/messageaction.cpp \
    widget/tool/chatactions/filetransferaction.cpp \
    widget/tool/chatactions/systemmessageaction.cpp \
    widget/tool/chatactions/actionaction.cpp \
    widget/maskablepixmapwidget.cpp \
    cameraworker.cpp \
    widget/videosurface.cpp
