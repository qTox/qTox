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


QT       += core gui network xml opengl sql svg
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
    src/chatlog/content/filetransferwidget.ui \
    src/widget/form/settings/advancedsettings.ui \
    src/android.ui
    
CONFIG   += c++11

# Rules for creating/updating {ts|qm}-files
include(translations/i18n.pri)
# Build all the qm files now, to make RCC happy
system($$fromfile(translations/i18n.pri, updateallqm))

RESOURCES += res.qrc \
    smileys/smileys.qrc

GIT_VERSION = $$system(git rev-parse HEAD 2> /dev/null || echo "built without git")
DEFINES += GIT_VERSION=\"\\\"$$quote($$GIT_VERSION)\\\"\"
# date works on linux/mac, but it would hangs qmake on windows
# This hack returns 0 on batch (windows), but executes "date +%s" or return 0 if it fails on bash (linux/mac)
TIMESTAMP = $$system($1 2>null||echo 0||a;rm null;date +%s||echo 0) # I'm so sorry
DEFINES += TIMESTAMP=$$TIMESTAMP
DEFINES += LOG_TO_FILE

contains(ENABLE_SYSTRAY_UNITY_BACKEND, YES) {
	DEFINES += ENABLE_SYSTRAY_UNITY_BACKEND

	INCLUDEPATH += "/usr/include/libappindicator-0.1"
	INCLUDEPATH += "/usr/include/gtk-2.0"
	INCLUDEPATH += "/usr/include/glib-2.0"
	INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
	INCLUDEPATH += "/usr/lib/i386-linux-gnu/glib-2.0/include"
	INCLUDEPATH += "/usr/include/cairo"
	INCLUDEPATH += "/usr/include/pango-1.0"
	INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/gtk-2.0/include"
	INCLUDEPATH += "/usr/lib/i386-linux-gnu/gtk-2.0/include"
	INCLUDEPATH += "/usr/include/gdk-pixbuf-2.0"
	INCLUDEPATH += "/usr/include/atk-1.0"
	INCLUDEPATH += "/usr/include/libdbusmenu-glib-0.4"

	LIBS += -lgobject-2.0 -lappindicator -lgtk-x11-2.0
}

contains(ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND, YES) {
	DEFINES += ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND

	INCLUDEPATH += "/usr/include/gtk-2.0"
	INCLUDEPATH += "/usr/include/glib-2.0"
	INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
	INCLUDEPATH += "/usr/lib/i386-linux-gnu/glib-2.0/include"
	INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/gtk-2.0/include"
	INCLUDEPATH += "/usr/lib/i386-linux-gnu/gtk-2.0/include"
	INCLUDEPATH += "/usr/include/gdk-pixbuf-2.0"
	INCLUDEPATH += "/usr/include/cairo"
	INCLUDEPATH += "/usr/include/pango-1.0"
	INCLUDEPATH += "/usr/include/atk-1.0"

	LIBS += -lglib-2.0 -lgdk_pixbuf-2.0 -lgio-2.0 -lcairo -lgtk-x11-2.0 -lgdk-x11-2.0 -lgobject-2.0

    SOURCES +=     src/platform/statusnotifier/closures.c \
    src/platform/statusnotifier/enums.c \
    src/platform/statusnotifier/statusnotifier.c

    HEADERS += src/platform/statusnotifier/closures.h \
    src/platform/statusnotifier/enums.h \
    src/platform/statusnotifier/interfaces.h \
    src/platform/statusnotifier/statusnotifier.h
}

android {
    ANDROID_TOOLCHAIN=/opt/android/toolchain-r9d-17/
    INCLUDEPATH += $$ANDROID_TOOLCHAIN/include/
    LIBS += -L$$PWD/libs/lib -L$$ANDROID_TOOLCHAIN/lib

    DISABLE_PLATFORM_EXT=YES
    DISABLE_FILTER_AUDIO=YES

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    contains(ANDROID_TARGET_ARCH,armeabi) {
        exists($$ANDROID_TOOLCHAIN/lib/libopenal.so) {
            ANDROID_EXTRA_LIBS = $$ANDROID_TOOLCHAIN/lib/libopenal.so
        } else {
        exists($$PWD/libs/lib/libopenal.so) {
            ANDROID_EXTRA_LIBS = $$PWD/libs/lib/libopenal.so
        } else {
            error(Can\'t find libopenal.so)
        }}
    }

}


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
	LIBS += -L$$PWD/libs/lib -ltoxav -ltoxcore -ltoxencryptsave -ltoxdns -lsodium -lvpx -lpthread
    LIBS += -L$$PWD/libs/lib -lopencv_core249 -lopencv_highgui249 -lopencv_imgproc249 -lOpenAL32 -lopus
    LIBS += -lopengl32 -lole32 -loleaut32 -luuid -lvfw32 -lws2_32 -liphlpapi -lz

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
        LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -ltoxencryptsave -ltoxdns -lsodium -lvpx -lopus -framework OpenAL -lopencv_core -lopencv_highgui
        contains(DEFINES, QTOX_PLATFORM_EXT) { LIBS += -framework IOKit -framework CoreFoundation }
        contains(DEFINES, QTOX_FILTER_AUDIO) { LIBS += -lfilteraudio }
    } else {
        android {
            LIBS += -ltoxcore -ltoxav -ltoxencryptsave -ltoxdns
            LIBS += -lopencv_videoio -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_androidcamera
            LIBS += -llibjpeg -llibwebp -llibpng -llibtiff -llibjasper -lIlmImf -lopencv_core
            LIBS += -lopus -lvpx -lsodium -lopenal
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
                LIBS = ./libs/lib/libtoxav.a ./libs/lib/libvpx.a ./libs/lib/libopus.a ./libs/lib/libtoxdns.a ./libs/lib/libtoxencryptsave.a ./libs/lib/libtoxcore.a ./libs/lib/libopenal.a ./libs/lib/libsodium.a ./libs/lib/libfilteraudio.a /usr/lib/libopencv_core.so /usr/lib/libopencv_highgui.so /usr/lib/libopencv_imgproc.so -lX11 -lXss
                contains(ENABLE_SYSTRAY_UNITY_BACKEND, YES) {
                    LIBS += -lgobject-2.0 -lappindicator -lgtk-x11-2.0
                }
                LIBS += -s
            }
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
    src/corestructs.h \
    src/coredefines.h \
    src/coreav.h \
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
    src/chatlog/chatlog.h \
    src/chatlog/chatline.h \
    src/chatlog/chatlinecontent.h \
    src/chatlog/chatlinecontentproxy.h \
    src/chatlog/content/text.h \
    src/chatlog/content/spinner.h \
    src/chatlog/content/filetransferwidget.h \
    src/chatlog/chatmessage.h \
    src/chatlog/content/image.h \
    src/chatlog/customtextdocument.h \
    src/widget/form/settings/advancedform.h \
    src/audio.h \
    src/chatlog/content/notificationicon.h \
    src/chatlog/content/timestamp.h \
    src/chatlog/documentcache.h \
    src/chatlog/pixmapcache.h \
    src/widget/callconfirmwidget.h \
    src/widget/systemtrayicon.h \
    src/widget/systemtrayicon_private.h \
    src/nexus.h \
    src/widget/gui.h \
    src/widget/androidgui.h \
    src/offlinemsgengine.h

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
    src/corestructs.cpp \
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
    src/chatlog/chatlog.cpp \
    src/chatlog/chatline.cpp \
    src/chatlog/chatlinecontent.cpp \
    src/chatlog/chatlinecontentproxy.cpp \
    src/chatlog/content/text.cpp \
    src/chatlog/content/spinner.cpp \
    src/chatlog/content/filetransferwidget.cpp \
    src/chatlog/chatmessage.cpp \
    src/chatlog/content/image.cpp \
    src/chatlog/customtextdocument.cpp\
    src/widget/form/settings/advancedform.cpp \
    src/audio.cpp \
    src/chatlog/content/notificationicon.cpp \
    src/chatlog/content/timestamp.cpp \
    src/chatlog/documentcache.cpp \
    src/chatlog/pixmapcache.cpp \
    src/widget/callconfirmwidget.cpp \
    src/widget/systemtrayicon.cpp \
    src/nexus.cpp \
    src/widget/gui.cpp \
    src/widget/androidgui.cpp \
    src/offlinemsgengine.cpp

contains(DEFINES, QTOX_FILTER_AUDIO) {
    HEADERS += src/audiofilterer.h
    SOURCES += src/audiofilterer.cpp
}

contains(DEFINES, QTOX_PLATFORM_EXT) {
    HEADERS += src/platform/timer.h
    SOURCES += src/platform/timer_osx.cpp \
               src/platform/timer_win.cpp \
               src/platform/timer_x11.cpp

    HEADERS += src/platform/autorun.h
    SOURCES += src/platform/autorun_win.cpp \
               src/platform/autorun_xdg.cpp \
               src/platform/autorun_osx.cpp
}
