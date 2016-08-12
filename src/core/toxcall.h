#ifndef TOXCALL_H
#define TOXCALL_H

#include <cstdint>
#include <QtGlobal>
#include <QMetaObject>

#include "src/audio/devices.h"
#include "src/core/indexedlist.h"

#include <tox/toxav.h>

#include <QDebug>

class QTimer;
class AudioFilterer;
class CoreVideoSource;
class CoreAV;

struct ToxCall
{
protected:
     ToxCall() = default;
     explicit ToxCall(uint32_t CallId);
     ~ToxCall() noexcept;
public:
     ToxCall(const ToxCall& other) = delete;
     ToxCall(ToxCall&& other) noexcept;

     inline operator uint32_t() {return callId;}
     ToxCall& operator=(const ToxCall& other) = delete;
     ToxCall& operator=(ToxCall&& other) noexcept;

     inline void playbackAudio(const qint16* pcm, quint32 frames,
                               quint8 channels, quint32 sampleRate)
     {
         qTox::Audio::Format fmt = qTox::Audio::Format::SINT16;

         size_t bytes = frames * channels * 2;
         char* data = new char[bytes];
         memcpy(data, reinterpret_cast<const char*>(pcm), bytes);
         audioRx.playback(data, fmt, frames, channels, sampleRate);
     }

public:
    uint32_t callId;
    bool inactive;
    bool muteMic;
    bool muteVol;

protected:
    qTox::Audio::Stream audioRx;
};

struct ToxFriendCall : public ToxCall
{
    ToxFriendCall() = default;
    ToxFriendCall(uint32_t FriendNum, bool VideoEnabled, CoreAV& av);
    ToxFriendCall(ToxFriendCall&& other) noexcept;
    ~ToxFriendCall();

    ToxFriendCall& operator=(ToxFriendCall&& other) noexcept;

    bool videoEnabled;
    bool nullVideoBitrate;
    CoreVideoSource* videoSource;
    TOXAV_FRIEND_CALL_STATE state;

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

    ToxGroupCall& operator=(ToxGroupCall&& other) noexcept;

    // If you add something here, don't forget to override the ctors and move operators!
};

#endif // TOXCALL_H

