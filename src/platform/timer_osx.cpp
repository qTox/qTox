/*
    Pidgin is the legal property of its developers, whose names are too numerous
    to list here.  Please refer to the COPYRIGHT file distributed with this
    source distribution (which can be found at
    <https://hg.pidgin.im/pidgin/main/file/13e4ae613a6a/COPYRIGHT> ).

    Copyright © 2006 by Richard Laager
    Copyright © 2014-2019 by The qTox Project Contributors

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
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

uint32_t Platform::getIdleTime()
{
    // https://hg.pidgin.im/pidgin/main/file/13e4ae613a6a/pidgin/gtkidle.c
    // relevant code introduced to Pidgin in:
    // https://hg.pidgin.im/pidgin/main/diff/8ff1c408ef3e/src/gtkidle.c
    static io_service_t service = 0;
    CFTypeRef property;
    uint64_t idleTime_ns = 0;

    if (!service) {
        mach_port_t master;
        IOMasterPort(MACH_PORT_NULL, &master);
        service = IOServiceGetMatchingService(master, IOServiceMatching("IOHIDSystem"));
    }

    property = IORegistryEntryCreateCFProperty(service, CFSTR("HIDIdleTime"), kCFAllocatorDefault, 0);
    CFNumberGetValue(static_cast<CFNumberRef>(property), kCFNumberSInt64Type, &idleTime_ns);
    CFRelease(property);

    return idleTime_ns / 1000000;
}
