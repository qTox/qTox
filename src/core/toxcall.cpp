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

ToxCall::ToxCall(bool VideoEnabled, CoreAV& av)
    : av{&av}
    , videoEnabled{VideoEnabled}
    , audioSource{Audio::getInstance().makeSource()}
{
}

/**
 * @brief ToxCall move constructor
 * @param other object moved from
 */
ToxCall::ToxCall(ToxCall&& other) noexcept
    : active{other.active}
    , av{other.av}
    , audioInConn{other.audioInConn}
    , muteMic{other.muteMic}
    , muteVol{other.muteVol}
    , videoSource{other.videoSource}
    , videoInConn{other.videoInConn}
    , videoEnabled{other.videoEnabled}
    , nullVideoBitrate{other.nullVideoBitrate}
    , audioSource{std::move(other.audioSource)}
{
    other.audioInConn = QMetaObject::Connection();
    other.videoInConn = QMetaObject::Connection();
    other.videoEnabled = false; // we don't need to subscribe video because other won't unsubscribe
    other.videoSource = nullptr;
    other.av = nullptr;
}

ToxCall::~ToxCall()
{
    QObject::disconnect(audioInConn);
    if (videoEnabled) {
        QObject::disconnect(videoInConn);
        CameraSource::getInstance().unsubscribe();
    }
}

/**
 * @brief ToxCall move assignement
 * @param other object moved from
 * @return object moved to
 */
ToxCall& ToxCall::operator=(ToxCall&& other) noexcept
{
    QObject::disconnect(audioInConn);

    audioInConn = other.audioInConn;
    other.audioInConn = QMetaObject::Connection();
    active = other.active;
    muteMic = other.muteMic;
    muteVol = other.muteVol;

    audioSource = std::move(other.audioSource);

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

ToxFriendCall::ToxFriendCall(uint32_t FriendNum, bool VideoEnabled, CoreAV& av)
    : ToxCall(VideoEnabled, av)
    , sink(Audio::getInstance().makeSink())
{
    // TODO(sudden6): move this to audio source
    audioInConn = QObject::connect(audioSource.get(), &IAudioSource::frameAvailable,
    [&av, FriendNum](const int16_t* pcm, size_t samples, uint8_t chans,
            uint32_t rate) {
        av.sendCallAudio(FriendNum, pcm, samples, chans, rate);
    });

    if (!audioInConn) {
       qDebug() << "Audio input connection not working";
    }

    // register video
    if (videoEnabled) {
        videoSource = new CoreVideoSource();
        CameraSource& source = CameraSource::getInstance();

        if (source.isNone()) {
            source.setupDefault();
        }
        source.subscribe();
        videoInConn = QObject::connect(&source, &VideoSource::frameAvailable,
                                       [&av, FriendNum](std::shared_ptr<VideoFrame> frame) {
                                           av.sendCallVideo(FriendNum, frame);
                                       });
        if (!videoInConn) {
            qDebug() << "Video connection not working";
        }
    }
}

ToxFriendCall::ToxFriendCall(ToxFriendCall &&other) noexcept
    : ToxCall(std::move(other))
    , sink{std::move(other.sink)}
{
}

ToxFriendCall& ToxFriendCall::operator=(ToxFriendCall &&other) noexcept
{
    ToxCall::operator=(std::move(other));
    sink = std::move(other.sink);

    return *this;
}

ToxFriendCall::~ToxFriendCall()
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

const std::unique_ptr<IAudioSink> &ToxFriendCall::getAudioSink() const
{
    return sink;
}

ToxGroupCall::ToxGroupCall(int GroupNum, CoreAV& av)
    : ToxCall(false, av)
{
    // register audio
    audioInConn = QObject::connect(audioSource.get(), &IAudioSource::frameAvailable,
    [&av, GroupNum](const int16_t* pcm, size_t samples, uint8_t chans,
            uint32_t rate) {
        av.sendGroupCallAudio(GroupNum, pcm, samples, chans, rate);
    });

    if (!audioInConn) {
       qDebug() << "Audio input connection not working";
    }
}

ToxGroupCall::ToxGroupCall(ToxGroupCall&& other) noexcept
    : ToxCall(std::move(other))
{
    peers = std::move(other.peers);
}

ToxGroupCall::~ToxGroupCall()
{
}

ToxGroupCall& ToxGroupCall::operator=(ToxGroupCall&& other) noexcept
{
    ToxCall::operator=(std::move(other));
    peers = std::move(other.peers);

    return *this;
}

void ToxGroupCall::removePeer(int peerId)
{
    const auto& source = peers.find(peerId);
    if(source == peers.cend()) {
        qDebug() << "Peer:" << peerId << "does not have a source, can't remove";
        return;
    }

    peers.erase(source);
}

void ToxGroupCall::addPeer(int peerId)
{
    auto& audio = Audio::getInstance();
    peers.emplace(peerId, std::unique_ptr<IAudioSink>(audio.makeSink()));
}

bool ToxGroupCall::havePeer(int peerId)
{
    const auto& source = peers.find(peerId);
    return source != peers.cend();
}

void ToxGroupCall::clearPeers()
{
    peers.clear();
}

const std::unique_ptr<IAudioSink> &ToxGroupCall::getAudioSink(int peerId)
{
    if(!havePeer(peerId)) {
        addPeer(peerId);
    }
    const auto& source = peers.find(peerId);
    return source->second;
}
