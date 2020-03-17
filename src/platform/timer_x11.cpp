/*
    Copyright © 2019 by The qTox Project Contributors

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

#include <QtCore/qsystemdetection.h>
#include "src/platform/timer.h"
#include "src/platform/x11_display.h"
#include <QDebug>
#include <X11/extensions/scrnsaver.h>

uint32_t Platform::getIdleTime()
{
    uint32_t idleTime = 0;

    Display* display = X11Display::lock();
    if (!display) {
        qDebug() << "XOpenDisplay failed";
        X11Display::unlock();
        return 0;
    }

    int32_t x11event = 0, x11error = 0;
    static int32_t hasExtension = XScreenSaverQueryExtension(display, &x11event, &x11error);
    if (hasExtension) {
        XScreenSaverInfo* info = XScreenSaverAllocInfo();
        if (info) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
            XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
#pragma GCC diagnostic pop
            idleTime = info->idle;
            XFree(info);
        } else
            qDebug() << "XScreenSaverAllocInfo() failed";
    }
    X11Display::unlock();
    return idleTime;
}
