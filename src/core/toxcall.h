#ifndef TOXCALL_H
#define TOXCALL_H

#include <QMap>
#include <QMetaObject>
#include <QtGlobal>
#include <cstdint>

#include <tox/toxav.h>

class QTimer;
class AudioFilterer;
class CoreVideoSource;
class CoreAV;

class ToxCall
{
protected:
    explicit ToxCall();
    ~ToxCall();

public:
    ToxCall(const ToxCall& other) = delete;
    ToxCall(ToxCall&& other) noexcept;

    ToxCall& operator=(const ToxCall& other) = delete;
    ToxCall& operator=(ToxCall&& other) noexcept;

    bool isInactive() const;
    bool isActive() const;
    void setActive(bool value);

    bool getMuteVol() const;
    void setMuteVol(bool value);

    bool getMuteMic() const;
    void setMuteMic(bool value);

protected:
    QMetaObject::Connection audioInConn;
    bool active;
    bool muteMic;
    bool muteVol;
    bool valid = true;
};

class ToxFriendCall : public ToxCall
{
public:
    ToxFriendCall() = default;
    ToxFriendCall(uint32_t friendId, bool VideoEnabled, CoreAV& av);
    ToxFriendCall(ToxFriendCall&& other) noexcept;
    ~ToxFriendCall();

    ToxFriendCall& operator=(ToxFriendCall&& other) noexcept;

    void startTimeout(uint32_t callId);
    void stopTimeout();

    bool getVideoEnabled() const;
    void setVideoEnabled(bool value);

    bool getNullVideoBitrate() const;
    void setNullVideoBitrate(bool value);

    CoreVideoSource* getVideoSource() const;

    TOXAV_FRIEND_CALL_STATE getState() const;
    void setState(const TOXAV_FRIEND_CALL_STATE& value);

    quint32 getAlSource() const;
    void setAlSource(const quint32& value);

private:
    quint32 alSource;
    bool videoEnabled;
    bool nullVideoBitrate;
    CoreVideoSource* videoSource;
    TOXAV_FRIEND_CALL_STATE state;

protected:
    CoreAV* av;
    QTimer* timeoutTimer;

private:
    static constexpr int CALL_TIMEOUT = 45000;
};

class ToxGroupCall : public ToxCall
{
public:
    ToxGroupCall() = default;
    ToxGroupCall(int GroupNum, CoreAV& av);
    ToxGroupCall(ToxGroupCall&& other) noexcept;
    ~ToxGroupCall();

    ToxGroupCall& operator=(ToxGroupCall&& other) noexcept;

    void removePeer(int peerId);

    QMap<int, quint32>& getPeers();

private:
    QMap<int, quint32> peers;

    // If you add something here, don't forget to override the ctors and move operators!
};

#endif // TOXCALL_H
