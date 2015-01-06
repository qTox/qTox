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


QT       += core gui network xml opengl sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET    = qtox
TEMPLATE  = app
FORMS    += \
    src/mainwindow.ui \
    src/widget/form/settings/generalsettings.ui \
    src/widget/form/settings/avsettings.ui \
    src/widget/form/settings/identitysettings.ui \
    src/widget/form/settings/privacysettings.ui \
    src/widget/form/loadhistorydialog.ui \
    src/widget/form/setpassworddialog.ui \
    src/widget/form/settings/advancedsettings.ui
    
CONFIG   += c++11

# Rules for creating/updating {ts|qm}-files
include(translations/i18n.pri)
# Build all the qm files now, to make RCC happy
system($$fromfile(translations/i18n.pri, updateallqm))

RESOURCES += res.qrc

GIT_VERSION = $$system(git rev-parse HEAD 2> /dev/null || echo "built without git")
DEFINES += GIT_VERSION=\"\\\"$$quote($$GIT_VERSION)\\\"\"
# date works on linux/mac, but it would hangs qmake on windows
# This hack returns 0 on batch (windows), but executes "date +%s" or return 0 if it fails on bash (linux/mac)
TIMESTAMP = $$system($1 2>null||echo 0||a;rm null;date +%s||echo 0) # I'm so sorry
DEFINES += TIMESTAMP=$$TIMESTAMP
DEFINES += LOG_TO_FILE

contains(DISABLE_PLATFORM_EXT, YES) {

} else {
    DEFINES += QTOX_PLATFORM_EXT
}

contains(DISABLE_FILTER_AUDIO, YES) {

} else {
     DEFINES += QTOX_FILTER_AUDIO
}

contains(JENKINS,YES) {
	INCLUDEPATH += ./libs/include/
} else {
	INCLUDEPATH += libs/include
}

# Rules for Windows, Mac OSX, and Linux
win32 {
    RC_FILE = windows/qtox.rc
    LIBS += -liphlpapi -L$$PWD/libs/lib -lsodium -ltoxav -ltoxcore -ltoxencryptsave -ltoxdns -lvpx -lpthread
    LIBS += -L$$PWD/libs/lib -lopencv_core248 -lopencv_highgui248 -lopencv_imgproc248 -lOpenAL32 -lopus
    LIBS += -lopengl32 -lole32 -loleaut32 -luuid -lvfw32 -ljpeg -ltiff -lpng -ljasper -lIlmImf -lHalf -lws2_32 -lz

    contains(DEFINES, QTOX_FILTER_AUDIO) {
        contains(STATICPKG, YES) {
            LIBS += -Wl,-Bstatic -lfilteraudio
        } else {
            LIBS += -lfilteraudio
        }
    }
} else {
    macx {
        BUNDLEID = im.tox.qtox
        ICON = img/icons/qtox.icns
        QMAKE_INFO_PLIST = osx/info.plist
        LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -ltoxencryptsave -ltoxdns -lsodium -lvpx -framework OpenAL -lopencv_core -lopencv_highgui
        contains(DEFINES, QTOX_PLATFORM_EXT) { LIBS += -framework IOKit -framework CoreFoundation }
        contains(DEFINES, QTOX_FILTER_AUDIO) { LIBS += -lfilteraudio }
    } else {
        # If we're building a package, static link libtox[core,av] and libsodium, since they are not provided by any package
        contains(STATICPKG, YES) {
            target.path = /usr/bin
            INSTALLS += target
            LIBS += -L$$PWD/libs/lib/ -lopus -lvpx -lopenal -Wl,-Bstatic -ltoxcore -ltoxav -ltoxencryptsave -ltoxdns -lsodium -lopencv_highgui -lopencv_imgproc -lopencv_core -lz -Wl,-Bdynamic
	        LIBS += -Wl,-Bstatic -ljpeg -ltiff -lpng -ljasper -lIlmImf -lIlmThread -lIex -ldc1394 -lraw1394 -lHalf -lz -llzma -ljbig
            LIBS += -Wl,-Bdynamic -lv4l1 -lv4l2 -lavformat -lavcodec -lavutil -lswscale -lusb-1.0
        } else {
            LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -ltoxencryptsave -ltoxdns -lvpx -lsodium -lopenal -lopencv_core -lopencv_highgui -lopencv_imgproc
        }

        contains(DEFINES, QTOX_PLATFORM_EXT) {
            LIBS += -lX11 -lXss
        }

        contains(DEFINES, QTOX_FILTER_AUDIO) {
            contains(STATICPKG, YES) {
                LIBS += -Wl,-Bstatic -lfilteraudio
            } else {
                LIBS += -lfilteraudio
            }
        }

        contains(JENKINS, YES) {
            LIBS = ./libs/lib/libtoxav.a ./libs/lib/libvpx.a ./libs/lib/libopus.a ./libs/lib/libtoxdns.a ./libs/lib/libtoxencryptsave.a ./libs/lib/libtoxcore.a ./libs/lib/libsodium.a ./libs/lib/libfilteraudio.a /usr/lib/libopencv_core.so /usr/lib/libopencv_highgui.so /usr/lib/libopencv_imgproc.so -lopenal -lX11 -lXss -s
        }
    }
}

HEADERS  += src/widget/form/addfriendform.h \
    src/widget/form/chatform.h \
    src/widget/form/groupchatform.h \
    src/widget/form/settingswidget.h \
    src/widget/form/settings/genericsettings.h \
    src/widget/form/settings/generalform.h \
    src/widget/form/settings/identityform.h \
    src/widget/form/settings/privacyform.h \
    src/widget/form/settings/avform.h \
    src/widget/form/filesform.h \
    src/widget/tool/chattextedit.h \
    src/widget/tool/friendrequestdialog.h \
    src/widget/friendwidget.h \
    src/widget/groupwidget.h \
    src/widget/widget.h \
    src/friend.h \
    src/group.h \
    src/grouplist.h \
    src/misc/settings.h \
    src/core.h \
    src/friendlist.h \
    src/misc/cdata.h \
    src/misc/cstring.h \
    src/video/camera.h \
    src/widget/netcamview.h \
    src/misc/smileypack.h \
    src/widget/emoticonswidget.h \
    src/misc/style.h \
    src/widget/adjustingscrollarea.h \
    src/widget/croppinglabel.h \
    src/widget/friendlistwidget.h \
    src/widget/genericchatroomwidget.h \
    src/widget/form/genericchatform.h \
    src/widget/tool/chatactions/chataction.h \
    src/widget/chatareawidget.h \
    src/filetransferinstance.h \
    src/corestructs.h \
    src/coredefines.h \
    src/coreav.h \
    src/widget/tool/chatactions/messageaction.h \
    src/widget/tool/chatactions/filetransferaction.h \
    src/widget/tool/chatactions/systemmessageaction.h \
    src/widget/tool/chatactions/actionaction.h \
    src/widget/tool/chatactions/alertaction.h \
    src/widget/maskablepixmapwidget.h \
    src/video/videosource.h \
    src/video/cameraworker.h \
    src/widget/videosurface.h \
    src/widget/form/loadhistorydialog.h \
    src/historykeeper.h \
    src/misc/db/genericddinterface.h \
    src/misc/db/plaindb.h \
    src/misc/db/encrypteddb.h \
    src/widget/form/setpassworddialog.h \
    src/widget/form/tabcompleter.h \
    src/video/videoframe.h \
    src/misc/flowlayout.h \
    src/ipc.h \
    src/widget/toxuri.h \
    src/toxdns.h \
    src/widget/toxsave.h \
    src/autoupdate.h \
    src/misc/serialize.h \
    src/widget/form/settings/advancedform.h \
    src/audio.h

SOURCES += \
    src/widget/form/addfriendform.cpp \
    src/widget/form/chatform.cpp \
    src/widget/form/groupchatform.cpp \
    src/widget/form/settingswidget.cpp \
    src/widget/form/settings/generalform.cpp \
    src/widget/form/settings/identityform.cpp \
    src/widget/form/settings/privacyform.cpp \
    src/widget/form/settings/avform.cpp \
    src/widget/form/filesform.cpp \
    src/widget/tool/chattextedit.cpp \
    src/widget/tool/friendrequestdialog.cpp \
    src/widget/friendwidget.cpp \
    src/widget/groupwidget.cpp \
    src/widget/widget.cpp \
    src/core.cpp \
    src/coreencryption.cpp \
    src/friend.cpp \
    src/friendlist.cpp \
    src/group.cpp \
    src/grouplist.cpp \
    src/main.cpp \
    src/misc/settings.cpp \
    src/misc/cdata.cpp \
    src/misc/cstring.cpp \
    src/video/camera.cpp \
    src/widget/netcamview.cpp \
    src/misc/smileypack.cpp \
    src/widget/emoticonswidget.cpp \
    src/misc/style.cpp \
    src/widget/adjustingscrollarea.cpp \
    src/widget/croppinglabel.cpp \
    src/widget/friendlistwidget.cpp \
    src/coreav.cpp \
    src/widget/genericchatroomwidget.cpp \
    src/widget/form/genericchatform.cpp \
    src/widget/tool/chatactions/chataction.cpp \
    src/widget/chatareawidget.cpp \
    src/filetransferinstance.cpp \
    src/corestructs.cpp \
    src/widget/tool/chatactions/messageaction.cpp \
    src/widget/tool/chatactions/filetransferaction.cpp \
    src/widget/tool/chatactions/systemmessageaction.cpp \
    src/widget/tool/chatactions/actionaction.cpp \
    src/widget/tool/chatactions/alertaction.cpp \
    src/widget/maskablepixmapwidget.cpp \
    src/video/cameraworker.cpp \
    src/widget/videosurface.cpp \
    src/widget/form/loadhistorydialog.cpp \
    src/historykeeper.cpp \
    src/misc/db/genericddinterface.cpp \
    src/misc/db/plaindb.cpp \
    src/misc/db/encrypteddb.cpp \
    src/widget/form/setpassworddialog.cpp \
    src/video/netvideosource.cpp \
    src/widget/form/tabcompleter.cpp \
    src/video/videoframe.cpp \
    src/misc/flowlayout.cpp \
    src/widget/toxuri.cpp \
    src/toxdns.cpp \
    src/ipc.cpp \
    src/widget/toxsave.cpp \    
    src/autoupdate.cpp \
    src/misc/serialize.cpp \
    src/widget/form/settings/advancedform.cpp \
    src/audio.cpp

contains(DEFINES, QTOX_FILTER_AUDIO) {
    HEADERS += src/audiofilterer.h
    SOURCES += src/audiofilterer.cpp
}

contains(DEFINES, QTOX_PLATFORM_EXT) {
    HEADERS += src/platform/timer.h
    SOURCES += src/platform/timer_osx.cpp \
               src/platform/timer_win.cpp \
               src/platform/timer_x11.cpp
}
