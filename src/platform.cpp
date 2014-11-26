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

u_int32_t Platform::getIdleTime()
{
    // http://qt-project.org/faq/answer/how_can_i_detect_a_period_of_no_user_interaction
    // Detecting global inactivity, like Skype, is possible but not via Qt:
    // http://stackoverflow.com/a/21905027/1497645
    // https://hg.pidgin.im/pidgin/main/file/13e4ae613a6a/pidgin/gtkidle.c
    u_int32_t idleTime = 0;

#if defined(Q_OS_WIN32)
    LASTINPUTINFO info = { 0 };
    if(GetLastInputInfo(&info))
        idleTime = info.dwTime / 1000;
#elif defined(__APPLE__) && defined(__MACH__)
    static io_service_t service = NULL;
    CFTypeRef property;
    u_int64_t idleTime_ns = 0;

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

