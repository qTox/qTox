#ifndef PLATFORM_H
#define PLATFORM_H

#include <sys/types.h>

/* Platform-dependent code */

namespace Platform
{
    u_int32_t getIdleTime();
}

#endif // PLATFORM_H
