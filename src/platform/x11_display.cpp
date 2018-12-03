/*
    Copyright © 2017-2018 by The qTox Project Contributors

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
#include "src/platform/x11_display.h"
#include <QMutex>
#include <X11/Xlib.h>

namespace Platform {

struct X11DisplayPrivate
{
    Display* display;
    QMutex mutex;

    X11DisplayPrivate()
        : display(XOpenDisplay(nullptr))
    {
    }
    ~X11DisplayPrivate()
    {
        if (display) {
            XCloseDisplay(display);
        }
    }
    static X11DisplayPrivate& getSingleInstance()
    {
        // object created on-demand
        static X11DisplayPrivate singleInstance;
        return singleInstance;
    }
};

Display* X11Display::lock()
{
    X11DisplayPrivate& singleInstance = X11DisplayPrivate::getSingleInstance();
    singleInstance.mutex.lock();
    return singleInstance.display;
}

void X11Display::unlock()
{
    X11DisplayPrivate::getSingleInstance().mutex.unlock();
}
}
