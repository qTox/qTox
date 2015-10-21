#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T14:07:35
#
#-------------------------------------------------

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
    src/android.ui \
    src/loginscreen.ui \
    src/mainwindow.ui \
    src/chatlog/content/filetransferwidget.ui \
    src/widget/form/profileform.ui \
    src/widget/form/loadhistorydialog.ui \
    src/widget/form/setpassworddialog.ui \
    src/widget/form/settings/aboutsettings.ui \
    src/widget/form/settings/advancedsettings.ui \
    src/widget/form/settings/avsettings.ui \
    src/widget/form/settings/generalsettings.ui \
    src/widget/form/settings/privacysettings.ui \
    src/widget/form/removefrienddialog.ui

CONFIG   += c++11

QMAKE_CXXFLAGS += -fno-exceptions

# Rules for creating/updating {ts|qm}-files
include(translations/i18n.pri)
# Build all the qm files now, to make RCC happy
system($$fromfile(translations/i18n.pri, updateallqm))

GIT_VERSION = $$system(git rev-parse HEAD 2> /dev/null || echo "built without git")
DEFINES += GIT_VERSION=\"\\\"$$quote($$GIT_VERSION)\\\"\"
# date works on linux/mac, but it would hangs qmake on windows
# This hack returns 0 on batch (windows), but executes "date +%s" or return 0 if it fails on bash (linux/mac)
TIMESTAMP = $$system($1 2>null||echo 0||a;rm null;date +%s||echo 0) # I'm so sorry
DEFINES += TIMESTAMP=$$TIMESTAMP
DEFINES += LOG_TO_FILE
DEFINES += QT_MESSAGELOGCONTEXT

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

    RESOURCES += android.qrc

    HEADERS += src/widget/androidgui.h

    SOURCES += src/widget/androidgui.cpp

    DISTFILES += \
        android/gradle/wrapper/gradle-wrapper.jar \
        android/AndroidManifest.xml \
        android/gradlew.bat \
        android/res/values/libs.xml \
        android/build.gradle \
        android/gradle/wrapper/gradle-wrapper.properties \
        android/gradlew
}

contains(DISABLE_PLATFORM_EXT, YES) {

} else {
    DEFINES += QTOX_PLATFORM_EXT
}

contains(DISABLE_FILTER_AUDIO, YES) {

} else {
     DEFINES += QTOX_FILTER_AUDIO
}

contains(HIGH_DPI, YES) {
    QT_DEVICE_PIXEL_RATIO= auto
    DEFINES += HIGH_DPI
}

contains(JENKINS,YES) {
    INCLUDEPATH += ./libs/include/
} else {
    INCLUDEPATH += libs/include
}

contains(DEFINES, QTOX_FILTER_AUDIO) {
    HEADERS += src/audio/audiofilterer.h
    SOURCES += src/audio/audiofilterer.cpp
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

# Rules for Windows, Mac OSX, and Linux
win32 {
    RC_FILE = windows/qtox.rc
    LIBS += -L$$PWD/libs/lib -ltoxav -ltoxcore -ltoxencryptsave -ltoxdns -lsodium -lvpx -lpthread
    LIBS += -L$$PWD/libs/lib -lavformat -lavdevice -lavcodec -lavutil -lswscale -lOpenAL32 -lopus
    LIBS += -lopengl32 -lole32 -loleaut32 -lvfw32 -lws2_32 -liphlpapi -lz -luuid
    LIBS += -lqrencode
    LIBS += -lstrmiids # For DirectShow
    contains(DEFINES, QTOX_FILTER_AUDIO) {
        contains(STATICPKG, YES) {
            LIBS += -Wl,-Bstatic -lfilteraudio
        } else {
            LIBS += -lfilteraudio
        }
    }
} else {
    macx {
        BUNDLEID = chat.tox.qtox
        ICON = img/icons/qtox.icns
        QMAKE_INFO_PLIST = osx/info.plist
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
        LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -ltoxencryptsave -ltoxdns -lsodium -lvpx -lopus -framework OpenAL -lavformat -lavdevice -lavcodec -lavutil -lswscale -mmacosx-version-min=10.7
        LIBS += -lqrencode
        contains(DEFINES, QTOX_PLATFORM_EXT) { LIBS += -framework IOKit -framework CoreFoundation }
        contains(DEFINES, QTOX_FILTER_AUDIO) { LIBS += -lfilteraudio }
    } else {
        android {
            LIBS += -ltoxcore -ltoxav -ltoxencryptsave -ltoxdns
            LIBS += -llibjpeg -llibwebp -llibpng -llibtiff -llibjasper -lIlmImf
            LIBS += -lopus -lvpx -lsodium -lopenal
        } else {
            # If we're building a package, static link libtox[core,av] and libsodium, since they are not provided by any package
            contains(STATICPKG, YES) {
                target.path = /usr/bin
                INSTALLS += target
                LIBS += -L$$PWD/libs/lib/ -lopus -lvpx -lopenal -Wl,-Bstatic -ltoxcore -ltoxav -ltoxencryptsave -ltoxdns -lsodium -lavformat -lavdevice -lavcodec -lavutil -lswscale -lz -Wl,-Bdynamic
                LIBS += -Wl,-Bstatic -ljpeg -ltiff -lpng -ljasper -lIlmImf -lIlmThread -lIex -ldc1394 -lraw1394 -lHalf -lz -llzma -ljbig
                LIBS += -Wl,-Bdynamic -lv4l1 -lv4l2 -lavformat -lavcodec -lavutil -lswscale -lusb-1.0
                LIBS += -lqrencode
            } else {
                LIBS += -L$$PWD/libs/lib/ -ltoxcore -ltoxav -ltoxencryptsave -ltoxdns -lvpx -lsodium -lopenal -lavformat -lavdevice -lavcodec -lavutil -lswscale
                LIBS += -lqrencode
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
                LIBS = ./libs/lib/libtoxav.a ./libs/lib/libvpx.a ./libs/lib/libopus.a ./libs/lib/libtoxdns.a ./libs/lib/libtoxencryptsave.a ./libs/lib/libtoxcore.a ./libs/lib/libopenal.a ./libs/lib/libsodium.a ./libs/lib/libfilteraudio.a ./libs/lib/libavformat-ffmpeg.so ./libs/lib/libavdevice-ffmpeg.so ./libs/lib/libavcodec-ffmpeg.so ./libs/lib/libavutil-ffmpeg.so ./libs/lib/libswscale-ffmpeg.so -ldl -lX11 -lXss -lqrencode
                contains(ENABLE_SYSTRAY_UNITY_BACKEND, YES) {
                    LIBS += -lgobject-2.0 -lappindicator -lgtk-x11-2.0
                }
                LIBS += -s
            }
        }
    }
}

# The systray Unity backend implements the system tray icon on Unity (Ubuntu) and GNOME desktops.
unix:!macx:!android {
contains(ENABLE_SYSTRAY_UNITY_BACKEND, YES) {
    DEFINES += ENABLE_SYSTRAY_UNITY_BACKEND

    INCLUDEPATH += "/usr/include/glib-2.0"
    INCLUDEPATH += "/usr/include/gtk-2.0"
    INCLUDEPATH += "/usr/include/atk-1.0"
    INCLUDEPATH += "/usr/include/cairo"
    INCLUDEPATH += "/usr/include/ffmpeg"
    INCLUDEPATH += "/usr/include/gdk-pixbuf-2.0"
    INCLUDEPATH += "/usr/include/libappindicator-0.1"
    INCLUDEPATH += "/usr/include/libdbusmenu-glib-0.4"
    INCLUDEPATH += "/usr/include/pango-1.0"
    equals(QT_ARCH, x86_64) {
        INCLUDEPATH += "/usr/lib64/glib-2.0/include"
        INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
        INCLUDEPATH += "/usr/lib64/gtk-2.0/include"
        INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/gtk-2.0/include"
    }
    else {
        INCLUDEPATH += "/usr/lib/glib-2.0/include"
        INCLUDEPATH += "/usr/lib/i386-linux-gnu/glib-2.0/include"
        INCLUDEPATH += "/usr/lib/gtk-2.0/include"
        INCLUDEPATH += "/usr/lib/i386-linux-gnu/gtk-2.0/include"
    }

    LIBS += -lgobject-2.0 -lappindicator -lgtk-x11-2.0
}
}

# The systray Status Notifier backend implements the system tray icon on KDE and compatible desktops
unix:!macx:!android {
contains(ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND, NO) {
} else {
    DEFINES += ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND

    INCLUDEPATH += "/usr/include/glib-2.0"
    INCLUDEPATH += "/usr/include/gtk-2.0"
    INCLUDEPATH += "/usr/include/atk-1.0"
    INCLUDEPATH += "/usr/include/cairo"
    INCLUDEPATH += "/usr/include/ffmpeg"
    INCLUDEPATH += "/usr/include/gdk-pixbuf-2.0"
    INCLUDEPATH += "/usr/include/pango-1.0"
    equals(QT_ARCH, x86_64) {
        INCLUDEPATH += "/usr/lib64/glib-2.0/include"
        INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
        INCLUDEPATH += "/usr/lib64/gtk-2.0/include"
        INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/gtk-2.0/include"
    }
    else {
        INCLUDEPATH += "/usr/lib/glib-2.0/include"
        INCLUDEPATH += "/usr/lib/i386-linux-gnu/glib-2.0/include"
        INCLUDEPATH += "/usr/lib/gtk-2.0/include"
        INCLUDEPATH += "/usr/lib/i386-linux-gnu/gtk-2.0/include"
    }


    LIBS += -lglib-2.0 -lgdk_pixbuf-2.0 -lgio-2.0 -lcairo -lgtk-x11-2.0 -lgdk-x11-2.0 -lgobject-2.0

    SOURCES +=     src/platform/statusnotifier/closures.c \
    src/platform/statusnotifier/enums.c \
    src/platform/statusnotifier/statusnotifier.c

    HEADERS += src/platform/statusnotifier/closures.h \
    src/platform/statusnotifier/enums.h \
    src/platform/statusnotifier/interfaces.h \
    src/platform/statusnotifier/statusnotifier.h
}
}

# The systray GTK backend implements a system tray icon compatible with many systems
unix:!macx:!android {
contains(ENABLE_SYSTRAY_GTK_BACKEND, NO) {
} else {
    DEFINES += ENABLE_SYSTRAY_GTK_BACKEND

    INCLUDEPATH += "/usr/include/glib-2.0"
    INCLUDEPATH += "/usr/include/gtk-2.0"
    INCLUDEPATH += "/usr/include/atk-1.0"
    INCLUDEPATH += "/usr/include/gdk-pixbuf-2.0"
    INCLUDEPATH += "/usr/include/cairo"
    INCLUDEPATH += "/usr/include/pango-1.0"
    equals(QT_ARCH, x86_64) {
        INCLUDEPATH += "/usr/lib64/glib-2.0/include"
        INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
        INCLUDEPATH += "/usr/lib64/gtk-2.0/include"
        INCLUDEPATH += "/usr/lib/x86_64-linux-gnu/gtk-2.0/include"
    }
    else {
        INCLUDEPATH += "/usr/lib/glib-2.0/include"
        INCLUDEPATH += "/usr/lib/i386-linux-gnu/glib-2.0/include"
        INCLUDEPATH += "/usr/lib/gtk-2.0/include"
        INCLUDEPATH += "/usr/lib/i386-linux-gnu/gtk-2.0/include"
    }


    LIBS += -lglib-2.0 -lgdk_pixbuf-2.0 -lgio-2.0 -lcairo -lgtk-x11-2.0 -lgdk-x11-2.0 -lgobject-2.0
}
}

!android {
    RESOURCES += res.qrc \
        smileys/smileys.qrc

    HEADERS  += \
        src/friend.h \
        src/group.h \
        src/grouplist.h \
        src/friendlist.h \
        src/persistence/smileypack.h \
        src/widget/emoticonswidget.h \
        src/widget/style.h \
        src/widget/tool/croppinglabel.h \
        src/widget/maskablepixmapwidget.h \
        src/video/videosurface.h \
        src/net/toxuri.h \
        src/net/toxdns.h \
        src/persistence/toxsave.h \
        src/persistence/serialize.h \
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
        src/widget/form/settings/aboutform.h \
        src/widget/form/settings/advancedform.h \
        src/chatlog/content/notificationicon.h \
        src/chatlog/content/timestamp.h \
        src/chatlog/documentcache.h \
        src/chatlog/pixmapcache.h \
        src/persistence/offlinemsgengine.h \
        src/widget/form/addfriendform.h \
        src/widget/form/chatform.h \
        src/widget/form/groupchatform.h \
        src/widget/form/settingswidget.h \
        src/widget/form/settings/genericsettings.h \
        src/widget/form/settings/generalform.h \
        src/widget/form/settings/privacyform.h \
        src/widget/form/settings/avform.h \
        src/widget/form/filesform.h \
        src/widget/form/profileform.h \
        src/widget/tool/chattextedit.h \
        src/widget/tool/friendrequestdialog.h \
        src/widget/friendwidget.h \
        src/widget/groupwidget.h \
        src/widget/widget.h \
        src/video/netcamview.h \
        src/widget/friendlistwidget.h \
        src/widget/genericchatroomwidget.h \
        src/widget/form/genericchatform.h \
        src/widget/tool/adjustingscrollarea.h \
        src/widget/form/loadhistorydialog.h \
        src/widget/form/setpassworddialog.h \
        src/widget/form/tabcompleter.h \
        src/net/autoupdate.h \
        src/widget/tool/callconfirmwidget.h \
        src/widget/systemtrayicon.h \
        src/widget/qrwidget.h \
        src/widget/systemtrayicon_private.h \
        src/widget/loginscreen.h \
        src/ipc.h

    SOURCES += \
        src/widget/form/addfriendform.cpp \
        src/widget/form/settingswidget.cpp \
        src/widget/form/settings/generalform.cpp \
        src/widget/form/settings/privacyform.cpp \
        src/widget/form/settings/avform.cpp \
        src/widget/form/profileform.cpp \
        src/widget/form/filesform.cpp \
        src/widget/tool/chattextedit.cpp \
        src/widget/tool/friendrequestdialog.cpp \
        src/widget/widget.cpp \
        src/video/netcamview.cpp \
        src/widget/friendlistwidget.cpp \
        src/widget/tool/adjustingscrollarea.cpp \
        src/widget/form/loadhistorydialog.cpp \
        src/widget/form/setpassworddialog.cpp \
        src/widget/form/tabcompleter.cpp \
        src/widget/flowlayout.cpp \
        src/net/autoupdate.cpp \
        src/widget/tool/callconfirmwidget.cpp \
        src/widget/systemtrayicon.cpp \
        src/widget/groupwidget.cpp \
        src/widget/friendwidget.cpp \
        src/widget/form/chatform.cpp \
        src/widget/form/groupchatform.cpp \
        src/widget/form/genericchatform.cpp \
        src/friend.cpp \
        src/friendlist.cpp \
        src/group.cpp \
        src/grouplist.cpp \
        src/persistence/smileypack.cpp \
        src/widget/emoticonswidget.cpp \
        src/widget/style.cpp \
        src/widget/tool/croppinglabel.cpp \
        src/widget/maskablepixmapwidget.cpp \
        src/video/videosurface.cpp \
        src/net/toxuri.cpp \
        src/net/toxdns.cpp \
        src/persistence/toxsave.cpp \
        src/persistence/serialize.cpp \
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
        src/widget/form/settings/aboutform.cpp \
        src/widget/form/settings/advancedform.cpp \
        src/chatlog/content/notificationicon.cpp \
        src/chatlog/content/timestamp.cpp \
        src/chatlog/documentcache.cpp \
        src/chatlog/pixmapcache.cpp \
        src/persistence/offlinemsgengine.cpp \
        src/widget/qrwidget.cpp \
        src/widget/genericchatroomwidget.cpp \
        src/widget/loginscreen.cpp \
        src/ipc.cpp
}

win32 {
    HEADERS += \
        src/platform/camera/directshow.h

    SOURCES += \
        src/platform/camera/directshow.cpp
}

unix:!macx {
    HEADERS += \
        src/platform/camera/v4l2.h

    SOURCES += \
        src/platform/camera/v4l2.cpp
}

macx {
    SOURCES += \
        src/platform/install_osx.cpp

    HEADERS += \
        src/platform/install_osx.h
}

SOURCES += \
    src/audio/audio.cpp \
    src/persistence/historykeeper.cpp \
    src/main.cpp \
    src/nexus.cpp \
    src/core/cdata.cpp \
    src/core/cstring.cpp \
    src/persistence/settings.cpp \
    src/persistence/db/genericddinterface.cpp \
    src/persistence/db/plaindb.cpp \
    src/persistence/db/encrypteddb.cpp \
    src/video/videoframe.cpp \
    src/widget/gui.cpp \
    src/net/toxme.cpp \
    src/core/core.cpp \
    src/core/coreav.cpp \
    src/core/coreencryption.cpp \
    src/core/corefile.cpp \
    src/core/corestructs.cpp \
    src/persistence/profilelocker.cpp \
    src/net/avatarbroadcaster.cpp \
    src/widget/tool/screenshotgrabber.cpp \
    src/widget/tool/screengrabberchooserrectitem.cpp \
    src/widget/tool/screengrabberoverlayitem.cpp \
    src/widget/tool/toolboxgraphicsitem.cpp \
    src/widget/tool/flyoutoverlaywidget.cpp \
    src/widget/form/settings/verticalonlyscroller.cpp \
    src/video/cameradevice.cpp \
    src/video/camerasource.cpp \
    src/video/corevideosource.cpp \
    src/core/toxid.cpp \
    src/persistence/profile.cpp \
    src/widget/translator.cpp \
    src/persistence/settingsserializer.cpp \
    src/widget/notificationscrollarea.cpp \
    src/widget/notificationedgewidget.cpp \
    src/widget/circlewidget.cpp \
    src/widget/genericchatitemwidget.cpp \
    src/widget/friendlistlayout.cpp \
    src/widget/genericchatitemlayout.cpp \
    src/widget/categorywidget.cpp \
    src/widget/contentlayout.cpp \
    src/widget/contentdialog.cpp \
    src/video/genericnetcamview.cpp \
    src/widget/tool/activatedialog.cpp \
    src/widget/tool/movablewidget.cpp \
    src/widget/tool/micfeedbackwidget.cpp \
    src/widget/tool/removefrienddialog.cpp \
    src/video/groupnetcamview.cpp

HEADERS += \
    src/audio/audio.h \
    src/core/core.h \
    src/core/coreav.h \
    src/core/coredefines.h \
    src/core/corefile.h \
    src/core/corestructs.h \
    src/persistence/historykeeper.h \
    src/nexus.h \
    src/core/cdata.h \
    src/core/cstring.h \
    src/persistence/settings.h \
    src/persistence/db/genericddinterface.h \
    src/persistence/db/plaindb.h \
    src/persistence/db/encrypteddb.h \
    src/video/videoframe.h \
    src/video/videosource.h \
    src/widget/gui.h \
    src/net/toxme.h \
    src/persistence/profilelocker.h \
    src/net/avatarbroadcaster.h \
    src/widget/tool/screenshotgrabber.h \
    src/widget/tool/screengrabberchooserrectitem.h \
    src/widget/tool/screengrabberoverlayitem.h \
    src/widget/tool/toolboxgraphicsitem.h \
    src/widget/tool/flyoutoverlaywidget.h \
    src/widget/form/settings/verticalonlyscroller.h \
    src/video/cameradevice.h \
    src/video/camerasource.h \
    src/video/corevideosource.h \
    src/video/videomode.h \
    src/core/toxid.h \
    src/persistence/profile.h \
    src/widget/translator.h \
    src/persistence/settingsserializer.h \
    src/widget/notificationscrollarea.h \
    src/widget/notificationedgewidget.h \
    src/widget/circlewidget.h \
    src/widget/genericchatitemwidget.h \
    src/widget/friendlistlayout.h \
    src/widget/genericchatitemlayout.h \
    src/widget/categorywidget.h \
    src/widget/contentlayout.h \
    src/widget/contentdialog.h \
    src/widget/tool/activatedialog.h \
    src/widget/tool/micfeedbackwidget.h \
    src/widget/tool/removefrienddialog.h \
    src/widget/tool/movablewidget.h \
    src/video/genericnetcamview.h \
    src/video/groupnetcamview.h
