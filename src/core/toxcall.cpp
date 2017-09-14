#include "src/core/toxcall.h"
#include "src/audio/audio.h"
#include "src/core/coreav.h"
#include "src/persistence/settings.h"
#include "src/video/camerasource.h"
#include "src/video/corevideosource.h"
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

/**
 * @var uint32_t ToxCall::callId
 * @brief Could be a friendNum or groupNum, must uniquely identify the call. Do not modify!
 *
 * @var bool ToxCall::inactive
 * @brief True while we're not participating. (stopped group call, ringing but hasn't started yet,
 * ...)
 *
 * @var bool ToxFriendCall::videoEnabled
 * @brief True if our user asked for a video call, sending and recieving.
 *
 * @var bool ToxFriendCall::nullVideoBitrate
 * @brief True if our video bitrate is zero, i.e. if the device is closed.
 *
 * @var TOXAV_FRIEND_CALL_STATE ToxFriendCall::state
 * @brief State of the peer (not ours!)
 *
 * @var QMap ToxGroupCall::peers
 * @brief Keeps sources for users in group calls.
 */

using namespace std;

ToxCall::ToxCall(uint32_t callId, bool videoEnabled, CoreAV& av)
    : callId{callId}
    , videoEnabled{videoEnabled}
    , av{&av}
{
    Audio& audio = Audio::getInstance();
    audio.subscribeInput();
    audio.subscribeOutput(alSource);

    audioInConn = QObject::connect(&Audio::getInstance(), &Audio::frameAvailable,
                        [&av, callId](const int16_t* pcm, size_t samples,
                                        uint8_t chans, uint32_t rate) {
                           av.sendCallAudio(callId, pcm, samples, chans, rate);
                        });
    if (!audioInConn) {
        qDebug() << "Audio connection not working";
    }

    if (videoEnabled) {
        videoSource = new CoreVideoSource();
        CameraSource& source = CameraSource::getInstance();

        if (source.isNone()) {
            source.setupDefault();
        }
        source.subscribe();
        videoInConn = QObject::connect(&source, &VideoSource::frameAvailable,
                        [&av, callId](shared_ptr<VideoFrame> frame) {
                         av.sendCallVideo(callId, frame);
                        });
        if (!videoInConn) {
            qDebug() << "Video connection not working";
        }
    }
}

ToxCall::ToxCall(ToxCall&& other) noexcept
    : audioInConn{other.audioInConn}
    , callId{other.callId}
    , alSource{other.alSource}
    , inactive{other.inactive}
    , muteMic{other.muteMic}
    , muteVol{other.muteVol}
    , videoInConn{other.videoInConn}
    , videoEnabled{other.videoEnabled}
    , nullVideoBitrate{other.nullVideoBitrate}
    , videoSource{other.videoSource}
    , av{other.av}
{
    other.audioInConn = QMetaObject::Connection();
    other.callId = numeric_limits<decltype(callId)>::max();
    other.alSource = 0;

    other.videoInConn = QMetaObject::Connection();
    other.videoEnabled = false;
    other.videoSource = nullptr;

    // required -> ownership of audio input is moved to new instance
    Audio::getInstance().subscribeInput();
    CameraSource::getInstance().subscribe();
}

ToxCall::~ToxCall()
{
    Audio& audio = Audio::getInstance();

    QObject::disconnect(audioInConn);
    audio.unsubscribeInput();
    audio.unsubscribeOutput(alSource);

    QObject::disconnect(videoInConn);
    CameraSource::getInstance().unsubscribe();
    if (videoEnabled) {
        // TODO: check if async is still needed
            // This destructor could be running in a toxav callback while holding toxav locks.
            // If the CameraSource thread calls toxav *_send_frame, we might deadlock the toxav and
            // CameraSource locks,
            // so we unsuscribe asynchronously, it's fine if the webcam takes a couple milliseconds more
            // to poweroff.
        QtConcurrent::run([]() { CameraSource::getInstance().unsubscribe(); });
        if (videoSource) {
            videoSource->setDeleteOnClose(true);
            videoSource = nullptr;
        }
    }
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
    Audio::getInstance().subscribeInput();

    videoInConn = other.videoInConn;
    other.videoInConn = QMetaObject::Connection();
    videoEnabled = other.videoEnabled;
    other.videoEnabled = false;
    nullVideoBitrate = other.nullVideoBitrate;
    videoSource = other.videoSource;
    other.videoSource = nullptr;
    av = other.av;
    other.av = nullptr;

    CameraSource::getInstance().subscribe();

    return *this;
}

void ToxFriendCall::startTimeout()
{
    if (!timeoutTimer) {
        timeoutTimer = new QTimer();
        // We might move, so we need copies of members. CoreAV won't move while we're alive
        CoreAV* avCopy = av;
        auto callIdCopy = callId;
        QObject::connect(timeoutTimer, &QTimer::timeout,
                         [avCopy, callIdCopy]() { avCopy->timeoutCall(callIdCopy); });
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
    : ToxCall(FriendNum, videoEnabled, av)
    , state{static_cast<TOXAV_FRIEND_CALL_STATE>(0)}
{

}

ToxFriendCall::ToxFriendCall(ToxFriendCall&& other) noexcept
    : ToxCall(move(other))
    , state{other.state}
    , timeoutTimer{other.timeoutTimer}
{
    other.timeoutTimer = nullptr;
}

ToxFriendCall::~ToxFriendCall()
{
    if (timeoutTimer)
        delete timeoutTimer;
}

ToxFriendCall& ToxFriendCall::operator=(ToxFriendCall&& other) noexcept
{
    ToxCall::operator=(move(other));
    state = other.state;
    timeoutTimer = other.timeoutTimer;
    other.timeoutTimer = nullptr;

    return *this;
}

ToxGroupCall::ToxGroupCall(int GroupNum, CoreAV& av)
    : ToxCall(static_cast<decltype(callId)>(GroupNum), false, av)
{

}

ToxGroupCall::ToxGroupCall(ToxGroupCall&& other) noexcept
    : ToxCall(move(other))
{
}

ToxGroupCall::~ToxGroupCall()
{
    Audio& audio = Audio::getInstance();

    for (quint32 v : peers) {
        audio.unsubscribeOutput(v);
    }
}

ToxGroupCall& ToxGroupCall::operator=(ToxGroupCall&& other) noexcept
{
    ToxCall::operator=(move(other));

    return *this;
}
