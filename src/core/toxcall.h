#ifndef TOXCALL_H
#define TOXCALL_H

#include <memory>
#include <QMap>
#include <QMetaObject>
#include <QtGlobal>
#include <cstdint>

#include <tox/toxav.h>

#include "src/audio/iaudiosource.h"
#include "src/audio/iaudiosink.h"

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
    ToxCall(ToxCall&& other) noexcept;

    ToxCall& operator=(const ToxCall& other) = delete;
    ToxCall& operator=(ToxCall&& other) noexcept;

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
    ToxFriendCall(ToxFriendCall&& other) noexcept;
    ToxFriendCall& operator=(ToxFriendCall&& other) noexcept;
    ~ToxFriendCall();

    void startTimeout(uint32_t callId);
    void stopTimeout();

    TOXAV_FRIEND_CALL_STATE getState() const;
    void setState(const TOXAV_FRIEND_CALL_STATE& value);

    const std::unique_ptr<IAudioSink> &getAudioSink() const;

protected:
    std::unique_ptr<QTimer> timeoutTimer;

private:
    TOXAV_FRIEND_CALL_STATE state{TOXAV_FRIEND_CALL_STATE_NONE};
    static constexpr int CALL_TIMEOUT = 45000;
    std::unique_ptr<IAudioSink> sink = nullptr;
    uint32_t friendId;
    QMetaObject::Connection audioSinkInvalid;
    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated();
};

class ToxGroupCall : public ToxCall
{
public:
    ToxGroupCall() = delete;
    ToxGroupCall(int GroupNum, CoreAV& av);
    ToxGroupCall(ToxGroupCall&& other) noexcept;
    ~ToxGroupCall();

    ToxGroupCall& operator=(ToxGroupCall&& other) noexcept;

    void removePeer(int peerId);
    void addPeer(int peerId);
    bool havePeer(int peerId);
    void clearPeers();
    const std::unique_ptr<IAudioSink> &getAudioSink(int peerId);

private:
    // If you add something here, don't forget to override the ctors and move operators!
    // need std::map here, because QMap needs a copy constructor

    std::map<int, std::unique_ptr<IAudioSink>> peers;
    std::map<int, QMetaObject::Connection> sinkInvalid;
    int groupId;

    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated(int peerId);
};

#endif // TOXCALL_H
