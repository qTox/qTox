#ifndef SYSTEMTRAYICON_PRIVATE_H
#define SYSTEMTRAYICON_PRIVATE_H

#include <QSystemTrayIcon>

#ifdef ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND
#ifdef signals
#undef signals
#endif
extern "C" {
    #include <glib.h>
    #include <glib-object.h>
    #include <gio/gio.h>
    #include <gdk-pixbuf/gdk-pixbuf.h>
    #include "src/platform/statusnotifier/statusnotifier.h"
}
#define signals public
#endif

#ifdef ENABLE_SYSTRAY_UNITY_BACKEND
#ifdef signals
#undef signals
#endif
extern "C" {
    #include <libappindicator/app-indicator.h>
    #include <gtk/gtk.h>
    #include <libdbusmenu-glib/server.h>
}
#define signals public
#endif

enum class SystrayBackendType
{
    Qt,
    KDE5,
#ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    Unity,
#endif
#ifdef ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND
    StatusNotifier,
#endif
};

union SystrayBackend
{
    QSystemTrayIcon *qt;
#ifdef ENABLE_SYSTRAY_UNITY_BACKEND
    AppIndicator *unity;
#endif
};


#endif // SYSTEMTRAYICON_PRIVATE_H
