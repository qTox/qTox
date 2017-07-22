/*
    Copyright © 2014 by The qTox Project Contributors

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
#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
#include "src/platform/x11_display.h"

Platform::X11Display Platform::X11Display::singleInstance;

Platform::X11Display::X11Display()
    : display(XOpenDisplay(nullptr))
{
}

Platform::X11Display::~X11Display()
{
    if (display)
        XCloseDisplay(display);
}

Display* Platform::X11Display::lock()
{
    singleInstance.mutex.lock();
    return singleInstance.display;
}

void Platform::X11Display::unlock()
{
    singleInstance.mutex.unlock();
}

#endif // Q_OS_UNIX && !defined(__APPLE__) && !defined(__MACH__)
