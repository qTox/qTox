#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T14:07:35
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

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
    core.h \
    friendlist.h \
    cdata.h \
    cstring.h \
    audiobuffer.h \
    widget/selfcamview.h \
    widget/videosurface.h \
    widget/camera.h \
    widget/netcamview.h

FORMS    += widget.ui

CONFIG += c++11

RESOURCES += \
    res.qrc

LIBS += -ltoxcore -ltoxav -lsodium -lvpx

#### Static linux build
#LIBS += -Wl,-Bstatic -ltoxcore -ltoxav -lsodium -lvpx -lopus -lgstaudiodecoder -lgstcamerabin -lgstmediacapture \
#     -lgstmediaplayer -lqgsttools_p -lgstaudio-0.10 -lgstinterfaces-0.10 -lgstvideo-0.10 -lgstpbutils-0.10 \
#        -lgstapp-0.10 -lgstbase-0.10 -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lxml2 \
#         -lqtaudio_alsa -lasound -lqtmultimedia_m3u \
#     -lqtaccessiblewidgets -lqconnmanbearer -lqgenericbearer -lqnmbearer \
#      -lqxcb -lX11-xcb -lXi -lxcb-render-util -lxcb-glx -lxcb-render -ldbus-1 \
#    -lxcb -lxcb-image -lxcb-icccm -lxcb-sync -lxcb-xfixes -lxcb-shm -lxcb-randr -lxcb-shape \
#    -lxcb-keysyms -lxcb-xkb -lfontconfig -lfreetype -lXrender -lXext -lX11 \
#    -lmtdev -lqdds -lqicns -lqico -lqjp2 -lqmng -lqtga -lqtiff -lqwbmp -lqwebp \
#   -lpng -lz -licui18n -licuuc -licudata -lm -ldl -lgthread-2.0 \
#     -pthread -lglib-2.0 -lrt -lGL -lpthread -Wl,-Bdynamic
#QMAKE_CXXFLAGS += -Os -flto -static-libstdc++ -static-libgcc

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
    cdata.cpp \
    cstring.cpp \
    audiobuffer.cpp \
    widget/selfcamview.cpp \
    widget/videosurface.cpp \
    widget/camera.cpp \
    widget/netcamview.cpp



### EXAMPLE BUILD SETTINGS FOR WINDOWS
#win32: LIBS += -L$$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/ -ltoxcore

#INCLUDEPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include
#DEPENDPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/toxcore.lib
#else:win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/libtoxcore.a

#win32: LIBS += -L$$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/ -ltoxav

#INCLUDEPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include
#DEPENDPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/toxav.lib
#else:win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/libtoxav.a

#win32: LIBS += -L$$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/ -lvpx

#INCLUDEPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include
#DEPENDPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/vpx.lib
#else:win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/libvpx.a


#win32: LIBS += -L$$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/ -lopus

#INCLUDEPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include
#DEPENDPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/opus.lib
#else:win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/libopus.a

#win32: LIBS += -lws2_32

#win32: LIBS += -L$$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/ -lsodium

#INCLUDEPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include
#DEPENDPATH += $$PWD/../../../../Downloads/libtoxcore-win32-i686/include

#win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/sodium.lib
#else:win32-g++: PRE_TARGETDEPS += $$PWD/../../../../Downloads/libtoxcore-win32-i686/lib/libsodium.a
