/*
    Copyright © 2016-2019 by The qTox Project Contributors

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
#include "src/platform/capslock.h"
#include "src/platform/x11_display.h"
#include <X11/XKBlib.h>
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut

bool Platform::capsLockEnabled()
{
    Display* d = X11Display::lock();
    bool caps_state = false;
    if (d) {
        unsigned n;
        XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
        caps_state = (n & 0x01) == 1;
    }
    X11Display::unlock();
    return caps_state;
}
