/*
    Pidgin is the legal property of its developers, whose names are too numerous
    to list here.  Please refer to the COPYRIGHT file distributed with this
    source distribution (which can be found at
    <https://hg.pidgin.im/pidgin/main/file/13e4ae613a6a/COPYRIGHT> ).

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#if defined(__APPLE__) && defined(__MACH__)
#include "src/platform/timer.h"
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>


uint32_t Platform::getIdleTime()
{
    // https://hg.pidgin.im/pidgin/main/file/13e4ae613a6a/pidgin/gtkidle.c
    static io_service_t service = 0;
    CFTypeRef property;
    uint64_t idleTime_ns = 0;

    if (!service)
    {
        mach_port_t master;
        IOMasterPort(MACH_PORT_NULL, &master);
        service = IOServiceGetMatchingService(master, IOServiceMatching("IOHIDSystem"));
    }

    property = IORegistryEntryCreateCFProperty(service, CFSTR("HIDIdleTime"), kCFAllocatorDefault, 0);
    CFNumberGetValue((CFNumberRef)property, kCFNumberSInt64Type, &idleTime_ns);
    CFRelease(property);

    return idleTime_ns / 1000000;
}

#endif  // defined(__APPLE__) && defined(__MACH__)
