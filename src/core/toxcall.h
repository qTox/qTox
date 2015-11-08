#ifndef TOXCALL_H
#define TOXCALL_H

#include <cstdint>
#include "src/core/indexedlist.h"

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/al.h>
 #include <AL/alc.h>
#endif

#include <tox/toxav.h>

class QTimer;
class AudioFilterer;
class CoreVideoSource;
class CoreAV;

struct ToxCall
{
protected:
     ToxCall() = default;
     ToxCall(uint32_t CallId);
     ~ToxCall();
public:
     ToxCall(const ToxCall& other) = delete;
     ToxCall(ToxCall&& other) noexcept;

     inline operator int() {return callId;}
     const ToxCall& operator=(const ToxCall& other) = delete;
     const ToxCall& operator=(ToxCall&& other) noexcept;

protected:
     QTimer* sendAudioTimer;

public:
    uint32_t callId; ///< Could be a friendNum or groupNum, must uniquely identify the call
    bool inactive; ///< True while we're not participating. (stopped group call, ringing but hasn't started yet, ...)
    bool muteMic;
    bool muteVol;
    ALuint alSource;

#ifdef QTOX_FILTER_AUDIO
    AudioFilterer* filterer;
#endif
};

struct ToxFriendCall : public ToxCall
{
    ToxFriendCall() = default;
    ToxFriendCall(uint32_t FriendNum, bool VideoEnabled, CoreAV& av);
    ToxFriendCall(ToxFriendCall&& other) noexcept;
    ~ToxFriendCall();

    const ToxFriendCall& operator=(ToxFriendCall&& other) noexcept;

    bool videoEnabled; ///< True if our user asked for a video call, sending and recving
    bool nullVideoBitrate; ///< True if our video bitrate is zero, i.e. if the device is closed
    CoreVideoSource* videoSource;
    TOXAV_FRIEND_CALL_STATE state; ///< State of the peer (not ours!)

    void startTimeout();
    void stopTimeout();

protected:
    CoreAV* av;
    QTimer* timeoutTimer;

private:
    static constexpr int CALL_TIMEOUT = 45000;
};

struct ToxGroupCall : public ToxCall
{
    ToxGroupCall() = default;
    ToxGroupCall(int GroupNum, CoreAV& av);
    ToxGroupCall(ToxGroupCall&& other) noexcept;

    const ToxGroupCall& operator=(ToxGroupCall&& other) noexcept;

    // If you add something here, don't forget to override the ctors and move operators!
};

#endif // TOXCALL_H

