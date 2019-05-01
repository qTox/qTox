#ifndef TOXCALL_H
#define TOXCALL_H

#include "src/audio/iaudiosink.h"
#include "src/audio/iaudiosource.h"
#include <src/core/toxpk.h>
#include <tox/toxav.h>

#include <QMap>
#include <QMetaObject>
#include <QtGlobal>

#include <cstdint>
#include <memory>

class QTimer;
class AudioFilterer;
class CoreVideoSource;
class CoreAV;

class ToxCall
{
protected:
    ToxCall() = delete;
    ToxCall(bool VideoEnabled, CoreAV& av);
    ~ToxCall();

public:
    ToxCall(const ToxCall& other) = delete;
    ToxCall(ToxCall&& other) = delete;

    ToxCall& operator=(const ToxCall& other) = delete;
    ToxCall& operator=(ToxCall&& other) = delete;

    bool isActive() const;
    void setActive(bool value);

    bool getMuteVol() const;
    void setMuteVol(bool value);

    bool getMuteMic() const;
    void setMuteMic(bool value);

    bool getVideoEnabled() const;
    void setVideoEnabled(bool value);

    bool getNullVideoBitrate() const;
    void setNullVideoBitrate(bool value);

    CoreVideoSource* getVideoSource() const;

protected:
    bool active{false};
    CoreAV* av{nullptr};
    // audio
    QMetaObject::Connection audioInConn;
    bool muteMic{false};
    bool muteVol{false};
    // video
    CoreVideoSource* videoSource{nullptr};
    QMetaObject::Connection videoInConn;
    bool videoEnabled{false};
    bool nullVideoBitrate{false};
    std::unique_ptr<IAudioSource> audioSource = nullptr;
    QMetaObject::Connection audioSrcInvalid;
};

class ToxFriendCall : public ToxCall
{
public:
    ToxFriendCall() = delete;
    ToxFriendCall(uint32_t friendId, bool VideoEnabled, CoreAV& av);
    ToxFriendCall(ToxFriendCall&& other) = delete;
    ToxFriendCall& operator=(ToxFriendCall&& other) = delete;
    ~ToxFriendCall();

    void startTimeout(uint32_t callId);
    void stopTimeout();

    TOXAV_FRIEND_CALL_STATE getState() const;
    void setState(const TOXAV_FRIEND_CALL_STATE& value);

    void playAudioBuffer(const int16_t* data, int samples, unsigned channels, int sampleRate) const;

protected:
    std::unique_ptr<QTimer> timeoutTimer;

private:
    QMetaObject::Connection audioSinkInvalid;
    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated();

private:
    TOXAV_FRIEND_CALL_STATE state{TOXAV_FRIEND_CALL_STATE_NONE};
    static constexpr int CALL_TIMEOUT = 45000;
    std::unique_ptr<IAudioSink> sink = nullptr;
    uint32_t friendId;
};

class ToxGroupCall : public ToxCall
{
public:
    ToxGroupCall() = delete;
    ToxGroupCall(int GroupNum, CoreAV& av);
    ToxGroupCall(ToxGroupCall&& other) = delete;
    ~ToxGroupCall();

    ToxGroupCall& operator=(ToxGroupCall&& other) = delete;
    void removePeer(ToxPk peerId);

    void playAudioBuffer(const ToxPk& peer, const int16_t* data, int samples, unsigned channels,
                         int sampleRate);

private:
    void addPeer(ToxPk peerId);
    bool havePeer(ToxPk peerId);
    void clearPeers();

    std::map<ToxPk, std::unique_ptr<IAudioSink>> peers;
    std::map<ToxPk, QMetaObject::Connection> sinkInvalid;
    int groupId;

    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated(ToxPk peerId);
};

#endif // TOXCALL_H
