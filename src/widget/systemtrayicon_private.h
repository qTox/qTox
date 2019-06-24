/*
    Copyright © 2015-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef SYSTEMTRAYICON_PRIVATE_H
#define SYSTEMTRAYICON_PRIVATE_H

#include <QSystemTrayIcon>

#ifdef ENABLE_SYSTRAY_STATUSNOTIFIER_BACKEND
#ifdef signals
#undef signals
#endif
extern "C" {
#include "src/platform/statusnotifier/statusnotifier.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
}
#define signals public
#endif

#ifdef ENABLE_SYSTRAY_UNITY_BACKEND
#ifdef signals
#undef signals
#endif
extern "C" {
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <libdbusmenu-glib/server.h>
}
#define signals public
#endif

#ifdef ENABLE_SYSTRAY_GTK_BACKEND
#ifdef signals
#undef signals
#endif
extern "C" {
#include <gtk/gtk.h>
}
#define signals public
#endif

enum class SystrayBackendType
{
    Qt,
    Unity,
    StatusNotifier,
    GTK
};

#endif // SYSTEMTRAYICON_PRIVATE_H
