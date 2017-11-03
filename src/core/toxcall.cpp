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

ToxCall::ToxCall()
    : active{false}
    , muteMic{false}
    , muteVol{false}
{
    Audio& audio = Audio::getInstance();
    audio.subscribeInput();
}

ToxCall::ToxCall(ToxCall&& other) noexcept
    : audioInConn{other.audioInConn}
    , active{other.active}
    , muteMic{other.muteMic}
    , muteVol{other.muteVol}
{
    other.audioInConn = QMetaObject::Connection();
    // invalidate object, all resources are moved
    other.valid = false;
}

ToxCall::~ToxCall()
{
    Audio& audio = Audio::getInstance();

    // only free resources if they weren't moved
    if (valid) {
        QObject::disconnect(audioInConn);
        audio.unsubscribeInput();
    }
}

ToxCall& ToxCall::operator=(ToxCall&& other) noexcept
{
    if (valid) {
        // if we're already valid, we need to cleanup our resources since we're going to inherit the resources
        // that are being moved in
        QObject::disconnect(audioInConn);
        Audio::getInstance().unsubscribeInput();
    }

    audioInConn = other.audioInConn;
    other.audioInConn = QMetaObject::Connection();
    active = other.active;
    muteMic = other.muteMic;
    muteVol = other.muteVol;
    // invalidate object, all resources are moved
    other.valid = false;

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

void ToxFriendCall::startTimeout(uint32_t callId)
{
    if (!timeoutTimer) {
        timeoutTimer = new QTimer();
        // We might move, so we need copies of members. CoreAV won't move while we're alive
        CoreAV* avCopy = av;
        QObject::connect(timeoutTimer, &QTimer::timeout,
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
    delete timeoutTimer;
    timeoutTimer = nullptr;
}

bool ToxFriendCall::getVideoEnabled() const
{
    return videoEnabled;
}

void ToxFriendCall::setVideoEnabled(bool value)
{
    videoEnabled = value;
}

bool ToxFriendCall::getNullVideoBitrate() const
{
    return nullVideoBitrate;
}

void ToxFriendCall::setNullVideoBitrate(bool value)
{
    nullVideoBitrate = value;
}

CoreVideoSource* ToxFriendCall::getVideoSource() const
{
    return videoSource;
}

TOXAV_FRIEND_CALL_STATE ToxFriendCall::getState() const
{
    return state;
}

void ToxFriendCall::setState(const TOXAV_FRIEND_CALL_STATE& value)
{
    state = value;
}

quint32 ToxFriendCall::getAlSource() const
{
    return alSource;
}

void ToxFriendCall::setAlSource(const quint32& value)
{
    alSource = value;
}

ToxFriendCall::ToxFriendCall(uint32_t friendId, bool VideoEnabled, CoreAV& av)
    : ToxCall()
    , videoEnabled{VideoEnabled}
    , nullVideoBitrate{false}
    , videoSource{nullptr}
    , state{static_cast<TOXAV_FRIEND_CALL_STATE>(0)}
    , av{&av}
    , timeoutTimer{nullptr}
{
    Audio& audio = Audio::getInstance();
    audioInConn = QObject::connect(&audio, &Audio::frameAvailable,
                                   [&av, friendId](const int16_t* pcm, size_t samples,
                                                   uint8_t chans, uint32_t rate) {
                                       av.sendCallAudio(friendId, pcm, samples, chans, rate);
                                   });
    if (!audioInConn) {
        qDebug() << "Audio connection not working";
    }

    // subscribe audio output
    audio.subscribeOutput(alSource);

    if (videoEnabled) {
        videoSource = new CoreVideoSource;
        CameraSource& source = CameraSource::getInstance();

        if (source.isNone())
            source.setupDefault();
        source.subscribe();
        QObject::connect(&source, &VideoSource::frameAvailable,
                         [friendId, &av](shared_ptr<VideoFrame> frame) {
                             av.sendCallVideo(friendId, frame);
                         });
    }
}

ToxFriendCall::ToxFriendCall(ToxFriendCall&& other) noexcept
    : ToxCall(move(other))
    , alSource{other.alSource}
    , videoEnabled{other.videoEnabled}
    , nullVideoBitrate{other.nullVideoBitrate}
    , videoSource{other.videoSource}
    , state{other.state}
    , av{other.av}
    , timeoutTimer{other.timeoutTimer}
{
    other.videoEnabled = false;
    other.videoSource = nullptr;
    other.timeoutTimer = nullptr;
    // TODO: wrong, because "0" could be a valid source id
    other.alSource = 0;
}

ToxFriendCall::~ToxFriendCall()
{
    if (timeoutTimer)
        delete timeoutTimer;

    if (videoEnabled) {
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
    if (valid) {
        Audio::getInstance().unsubscribeOutput(alSource);
    }
}

ToxFriendCall& ToxFriendCall::operator=(ToxFriendCall&& other) noexcept
{
    ToxCall::operator=(move(other));
    videoEnabled = other.videoEnabled;
    other.videoEnabled = false;
    videoSource = other.videoSource;
    other.videoSource = nullptr;
    state = other.state;
    timeoutTimer = other.timeoutTimer;
    other.timeoutTimer = nullptr;
    av = other.av;
    nullVideoBitrate = other.nullVideoBitrate;
    alSource = other.alSource;
    // TODO: wrong, because "0" could be a valid source id
    other.alSource = 0;

    return *this;
}

ToxGroupCall::ToxGroupCall(int GroupNum, CoreAV& av)
    : ToxCall()
{
    static_assert(
        numeric_limits<uint32_t>::max() >= numeric_limits<decltype(GroupNum)>::max(),
        "The callId must be able to represent any group number, change its type if needed");

    audioInConn = QObject::connect(&Audio::getInstance(), &Audio::frameAvailable,
                                   [&av, GroupNum](const int16_t* pcm, size_t samples,
                                                   uint8_t chans, uint32_t rate) {
                                       av.sendGroupCallAudio(GroupNum, pcm, samples, chans, rate);
                                   });
}

ToxGroupCall::ToxGroupCall(ToxGroupCall&& other) noexcept
    : ToxCall(move(other))
    , peers{other.peers}
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
    ToxCall::operator=(move(other));
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
