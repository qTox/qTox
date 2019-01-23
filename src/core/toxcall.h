#ifndef TOXCALL_H
#define TOXCALL_H

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

    quint32 getAlSource() const;
    void setAlSource(const quint32& value);

protected:
    std::unique_ptr<QTimer> timeoutTimer;

private:
    TOXAV_FRIEND_CALL_STATE state{TOXAV_FRIEND_CALL_STATE_NONE};
    static constexpr int CALL_TIMEOUT = 45000;
    quint32 alSource{0};
};

class ToxGroupCall : public ToxCall
{
public:
    ToxGroupCall() = delete;
    ToxGroupCall(int GroupNum, CoreAV& av);
    ToxGroupCall(ToxGroupCall&& other) noexcept;
    ~ToxGroupCall();

    ToxGroupCall& operator=(ToxGroupCall&& other) noexcept;

    void removePeer(ToxPk peerId);
    void addPeer(ToxPk peerId);
    bool havePeer(ToxPk peerId);
    void clearPeers();

    quint32 getAlSource(ToxPk peer);

private:
    QMap<ToxPk, quint32> peers;

    // If you add something here, don't forget to override the ctors and move operators!
};

#endif // TOXCALL_H
