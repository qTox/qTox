/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

#include "platform.h"
#include <QDebug>
#if defined(Q_OS_WIN32)
#include <Windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#else // Q_OS_UNIX
#include <X11/extensions/scrnsaver.h>
#endif

uint32_t Platform::getIdleTime()
{
    // http://qt-project.org/faq/answer/how_can_i_detect_a_period_of_no_user_interaction
    // Detecting global inactivity, like Skype, is possible but not via Qt:
    // http://stackoverflow.com/a/21905027/1497645
    // https://hg.pidgin.im/pidgin/main/file/13e4ae613a6a/pidgin/gtkidle.c
    uint32_t idleTime = 0;

#if defined(Q_OS_WIN32)
    LASTINPUTINFO info = { 0 };
    if(GetLastInputInfo(&info))
        idleTime = info.dwTime / 1000;
#elif defined(__APPLE__) && defined(__MACH__)
    static io_service_t service = NULL;
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

    idleTime = idleTime_ns / 1000000;
#else   // Q_OS_UNIX
    Display *display = XOpenDisplay(NULL);
    if(!display)
    {
        qDebug() << "XOpenDisplay(NULL) failed";
        return 0;
    }

    int32_t x11event = 0, x11error = 0;
    static int32_t hasExtension = XScreenSaverQueryExtension(display, &x11event, &x11error);
    if(hasExtension)
    {
        XScreenSaverInfo *info = XScreenSaverAllocInfo();
        if(info)
        {
            XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
            idleTime = info->idle;
            XFree(info);
        }
        else
            qDebug() << "XScreenSaverAllocInfo() failed";
    }
    XCloseDisplay(display);
#endif
    return idleTime;
}

