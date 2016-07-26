#include "src/audio/audio.h"
#include "src/core/toxcall.h"
#include "src/core/coreav.h"
#include "src/persistence/settings.h"
#include "src/video/camerasource.h"
#include "src/video/corevideosource.h"
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

/**
@var uint32_t ToxCall::callId
@brief Could be a friendNum or groupNum, must uniquely identify the call. Do not modify!

@var bool ToxCall::inactive
@brief True while we're not participating. (stopped group call, ringing but hasn't started yet, ...)

@var bool ToxCall::videoEnabled
@brief True if our user asked for a video call, sending and recieving.

@var bool ToxCall::nullVideoBitrate
@brief True if our video bitrate is zero, i.e. if the device is closed.

@var TOXAV_FRIEND_CALL_STATE ToxCall::state
@brief State of the peer (not ours!)
*/

using namespace std;

ToxCall::ToxCall(uint32_t CallId)
    : callId{CallId}, alSource{0},
      inactive{true}, muteMic{false}, muteVol{false}
{
    Audio& audio = Audio::getInstance();
    audio.subscribeInput();
    audio.subscribeOutput(alSource);
}

ToxCall::ToxCall(ToxCall&& other) noexcept
    : audioInConn{other.audioInConn}, callId{other.callId}, alSource{other.alSource},
      inactive{other.inactive}, muteMic{other.muteMic}, muteVol{other.muteVol}
{
    other.audioInConn = QMetaObject::Connection();
    other.callId = numeric_limits<decltype(callId)>::max();
    other.alSource = 0;

    // required -> ownership of audio input is moved to new instance
    Audio& audio = Audio::getInstance();
    audio.subscribeInput();
}

ToxCall::~ToxCall()
{
    Audio& audio = Audio::getInstance();

    QObject::disconnect(audioInConn);
    audio.unsubscribeInput();
    audio.unsubscribeOutput(alSource);
}

ToxCall& ToxCall::operator=(ToxCall&& other) noexcept
{
    audioInConn = other.audioInConn;
    other.audioInConn = QMetaObject::Connection();
    callId = other.callId;
    other.callId = numeric_limits<decltype(callId)>::max();
    inactive = other.inactive;
    muteMic = other.muteMic;
    muteVol = other.muteVol;
    alSource = other.alSource;
    other.alSource = 0;

    // required -> ownership of audio input is moved to new instance
    Audio& audio = Audio::getInstance();
    audio.subscribeInput();

    return *this;
}

void ToxFriendCall::startTimeout()
{
    if (!timeoutTimer)
    {
        timeoutTimer = new QTimer();
        // We might move, so we need copies of members. CoreAV won't move while we're alive
        CoreAV* avCopy = av;
        auto callIdCopy = callId;
        QObject::connect(timeoutTimer, &QTimer::timeout, [avCopy, callIdCopy](){
           avCopy->timeoutCall(callIdCopy);
        });
    }

    if (!timeoutTimer->isActive())
        timeoutTimer->start(CALL_TIMEOUT);
}

void ToxFriendCall::stopTimeout()
{
    if (!timeoutTimer)
        return;

    timeoutTimer->stop();
    delete timeoutTimer;
    timeoutTimer = nullptr;
}

ToxFriendCall::ToxFriendCall(uint32_t FriendNum, bool VideoEnabled, CoreAV& av)
    : ToxCall(FriendNum),
      videoEnabled{VideoEnabled}, nullVideoBitrate{false}, videoSource{nullptr},
      state{static_cast<TOXAV_FRIEND_CALL_STATE>(0)},
      av{&av}, timeoutTimer{nullptr}
{
    audioInConn = QObject::connect(&Audio::getInstance(), &Audio::frameAvailable,
                     [&av,FriendNum](const int16_t *pcm, size_t samples, uint8_t chans, uint32_t rate)
    {
        av.sendCallAudio(FriendNum, pcm, samples, chans, rate);
    });

    if (videoEnabled)
    {
        videoSource = new CoreVideoSource;
        CameraSource& source = CameraSource::getInstance();

        if (!source.isOpen())
            source.open();
        source.subscribe();
        QObject::connect(&source, &VideoSource::frameAvailable,
                [FriendNum,&av](shared_ptr<VideoFrame> frame){av.sendCallVideo(FriendNum,frame);});
    }
}

ToxFriendCall::ToxFriendCall(ToxFriendCall&& other) noexcept
    : ToxCall(move(other)),
      videoEnabled{other.videoEnabled}, nullVideoBitrate{other.nullVideoBitrate},
      videoSource{other.videoSource}, state{other.state},
      av{other.av}, timeoutTimer{other.timeoutTimer}
{
    other.videoEnabled = false;
    other.videoSource = nullptr;
    other.timeoutTimer = nullptr;
}

ToxFriendCall::~ToxFriendCall()
{
    if (timeoutTimer)
        delete timeoutTimer;

    if (videoEnabled)
    {
        // This destructor could be running in a toxav callback while holding toxav locks.
        // If the CameraSource thread calls toxav *_send_frame, we might deadlock the toxav and CameraSource locks,
        // so we unsuscribe asynchronously, it's fine if the webcam takes a couple milliseconds more to poweroff.
        QtConcurrent::run([](){CameraSource::getInstance().unsubscribe();});
        if (videoSource)
        {
            videoSource->setDeleteOnClose(true);
            videoSource = nullptr;
        }
    }
}

ToxFriendCall& ToxFriendCall::operator=(ToxFriendCall&& other) noexcept
{
    ToxCall::operator =(move(other));
    videoEnabled = other.videoEnabled;
    other.videoEnabled = false;
    videoSource = other.videoSource;
    other.videoSource = nullptr;
    state = other.state;
    timeoutTimer = other.timeoutTimer;
    other.timeoutTimer = nullptr;
    av = other.av;
    nullVideoBitrate = other.nullVideoBitrate;

    return *this;
}

ToxGroupCall::ToxGroupCall(int GroupNum, CoreAV &av)
    : ToxCall(static_cast<decltype(callId)>(GroupNum))
{
    static_assert(numeric_limits<decltype(callId)>::max() >= numeric_limits<decltype(GroupNum)>::max(),
                  "The callId must be able to represent any group number, change its type if needed");

    audioInConn = QObject::connect(&Audio::getInstance(), &Audio::frameAvailable,
                    [&av,GroupNum](const int16_t *pcm, size_t samples, uint8_t chans, uint32_t rate)
    {
        av.sendGroupCallAudio(GroupNum, pcm, samples, chans, rate);
    });
}

ToxGroupCall::ToxGroupCall(ToxGroupCall&& other) noexcept
    : ToxCall(move(other))
{
}

ToxGroupCall &ToxGroupCall::operator=(ToxGroupCall &&other) noexcept
{
    ToxCall::operator =(move(other));

    return *this;
}
