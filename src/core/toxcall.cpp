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

ToxCall::ToxCall(uint32_t CallId, bool VideoEnabled, CoreAV& av)
    : videoEnabled{VideoEnabled}
    , av{&av}
{
    Audio& audio = Audio::getInstance();
    audio.subscribeInput();
    audio.subscribeOutput(alSource);

    audioInConn = QObject::connect(&Audio::getInstance(), &Audio::frameAvailable,
                                   [&av, CallId](const int16_t* pcm, size_t samples, uint8_t chans,
                                                 uint32_t rate) {
                                       av.sendCallAudio(CallId, pcm, samples, chans, rate);
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
                                       [&av, CallId](std::shared_ptr<VideoFrame> frame) {
                                           av.sendCallVideo(CallId, frame);
                                       });
        if (!videoInConn) {
            qDebug() << "Video connection not working";
        }
    }
}

ToxCall::ToxCall(ToxCall&& other) noexcept : audioInConn{other.audioInConn},
                                             alSource{other.alSource},
                                             active{other.active},
                                             muteMic{other.muteMic},
                                             muteVol{other.muteVol},
                                             videoInConn{other.videoInConn},
                                             videoEnabled{other.videoEnabled},
                                             nullVideoBitrate{other.nullVideoBitrate},
                                             videoSource{other.videoSource},
                                             av{other.av}
{
    Audio& audio = Audio::getInstance();
    audio.subscribeInput();
    other.audioInConn = QMetaObject::Connection();
    other.alSource = 0;
    other.videoInConn = QMetaObject::Connection();
    other.videoEnabled = false; // we don't need to subscribe video because other won't unsubscribe
    other.videoSource = nullptr;
    other.av = nullptr;
}

ToxCall::~ToxCall()
{
    Audio& audio = Audio::getInstance();

    QObject::disconnect(audioInConn);
    audio.unsubscribeInput();
    audio.unsubscribeOutput(alSource);
    if (videoEnabled) {
        QObject::disconnect(videoInConn);
        CameraSource::getInstance().unsubscribe();
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
    QObject::disconnect(audioInConn);

    audioInConn = other.audioInConn;
    other.audioInConn = QMetaObject::Connection();
    active = other.active;
    muteMic = other.muteMic;
    muteVol = other.muteVol;

    alSource = other.alSource;
    other.alSource = 0;

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

    return *this;
}

bool ToxCall::isActive() const
{
    return active;
}

void ToxCall::setActive(bool value)
{
    active = value;
}

bool ToxCall::getMuteVol() const
{
    return muteVol;
}

void ToxCall::setMuteVol(bool value)
{
    muteVol = value;
}

bool ToxCall::getMuteMic() const
{
    return muteMic;
}

void ToxCall::setMuteMic(bool value)
{
    muteMic = value;
}

bool ToxCall::getVideoEnabled() const
{
    return videoEnabled;
}

void ToxCall::setVideoEnabled(bool value)
{
    videoEnabled = value;
}

bool ToxCall::getNullVideoBitrate() const
{
    return nullVideoBitrate;
}

void ToxCall::setNullVideoBitrate(bool value)
{
    nullVideoBitrate = value;
}

CoreVideoSource* ToxCall::getVideoSource() const
{
    return videoSource;
}

quint32 ToxCall::getAlSource() const
{
    return alSource;
}

void ToxCall::setAlSource(const quint32& value)
{
    alSource = value;
}

ToxFriendCall::ToxFriendCall(uint32_t FriendNum, bool VideoEnabled, CoreAV& av)
    : ToxCall(FriendNum, VideoEnabled, av)
{
}

void ToxFriendCall::startTimeout(uint32_t callId)
{
    if (!timeoutTimer) {
        timeoutTimer = std::unique_ptr<QTimer>{new QTimer{}};
        // We might move, so we need copies of members. CoreAV won't move while we're alive
        CoreAV* avCopy = av;
        QObject::connect(timeoutTimer.get(), &QTimer::timeout,
                         [avCopy, callId]() { avCopy->timeoutCall(callId); });
    }

    if (!timeoutTimer->isActive())
        timeoutTimer->start(CALL_TIMEOUT);
}

void ToxFriendCall::stopTimeout()
{
    if (!timeoutTimer)
        return;

    timeoutTimer->stop();
    timeoutTimer.reset();
}

TOXAV_FRIEND_CALL_STATE ToxFriendCall::getState() const
{
    return state;
}

void ToxFriendCall::setState(const TOXAV_FRIEND_CALL_STATE& value)
{
    state = value;
}

ToxGroupCall::ToxGroupCall(int GroupNum, CoreAV& av)
    : ToxCall(static_cast<uint32_t>(GroupNum), false, av)
{
}

ToxGroupCall::ToxGroupCall(ToxGroupCall&& other) noexcept : ToxCall(std::move(other)), peers{other.peers}
{
    // all peers were moved, this ensures audio output is unsubscribed only once
    other.peers.clear();
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
    ToxCall::operator=(std::move(other));
    peers = other.peers;
    other.peers.clear();

    return *this;
}

void ToxGroupCall::removePeer(int peerId)
{
    peers.remove(peerId);
}

QMap<int, quint32>& ToxGroupCall::getPeers()
{
    return peers;
}
