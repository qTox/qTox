#ifndef COREAV_H
#define COREAV_H

#include <QHash>
#include <tox/toxav.h>
#include "video/netvideosource.h"

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
    ToxAvCSettings codecSettings;
    QTimer *sendAudioTimer, *sendVideoTimer;
    int callId;
    int friendId;
    bool videoEnabled;
    bool active;
    bool muteMic;
    bool muteVol;
    ALuint alSource;
    NetVideoSource videoSource;
};

struct ToxGroupCall
{
    ToxAvCSettings codecSettings;
    QTimer *sendAudioTimer;
    int groupId;
    bool active = false;
    bool muteMic;
    bool muteVol;
    QHash<int, ALuint> alSources;
};

#endif // COREAV_H
