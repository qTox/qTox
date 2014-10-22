#ifndef COREAV_H
#define COREAV_H

#include <tox/toxav.h>
#include "netvideosource.h"

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/al.h>
 #include <AL/alc.h>
#endif

class QTimer;

struct ToxCall
{
public:
    ToxAvCSettings codecSettings;
    QTimer *sendAudioTimer, *sendVideoTimer;
    int callId;
    int friendId;
    bool videoEnabled;
    bool active;
    bool muteMic;
    ALuint alSource;
    NetVideoSource videoSource;
};

#endif // COREAV_H
