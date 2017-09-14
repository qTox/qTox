#ifndef TOXCALL_H
#define TOXCALL_H

#include <QMap>
#include <QMetaObject>
#include <QtGlobal>
#include <cstdint>

#include "src/core/indexedlist.h"

#include <tox/toxav.h>

class QTimer;
class AudioFilterer;
class CoreVideoSource;
class CoreAV;

struct ToxCall
{
protected:
    ToxCall() = default;
    ToxCall(uint32_t CallId, bool VideoEnabled, CoreAV& av);
    ~ToxCall();

public:
    ToxCall(const ToxCall& other) = delete;
    ToxCall(ToxCall&& other) noexcept;

    inline operator int()
    {
        return callId;
    }
    ToxCall& operator=(const ToxCall& other) = delete;
    ToxCall& operator=(ToxCall&& other) noexcept;

protected:
    QMetaObject::Connection audioInConn;
    QMetaObject::Connection videoInConn;
    CoreAV* av{nullptr};

public:
    uint32_t callId{0};
    // audio
    quint32 alSource{0};
    bool inactive{true};
    bool muteMic{false};
    bool muteVol{false};
    // video
    bool videoEnabled{false};
    bool nullVideoBitrate{false};
    CoreVideoSource* videoSource{nullptr};
};

struct ToxFriendCall : public ToxCall
{
    ToxFriendCall() = default;
    ToxFriendCall(uint32_t FriendNum, bool VideoEnabled, CoreAV& av);
    ToxFriendCall(ToxFriendCall&& other) noexcept;
    ~ToxFriendCall();

    ToxFriendCall& operator=(ToxFriendCall&& other) noexcept;

    TOXAV_FRIEND_CALL_STATE state{TOXAV_FRIEND_CALL_STATE_NONE};

    void startTimeout();
    void stopTimeout();

protected:
    QTimer* timeoutTimer{nullptr};

private:
    static constexpr int CALL_TIMEOUT = 45000;
};

struct ToxGroupCall : public ToxCall
{
    ToxGroupCall() = default;
    ToxGroupCall(int GroupNum, CoreAV& av);
    ToxGroupCall(ToxGroupCall&& other) noexcept;
    ~ToxGroupCall();

    ToxGroupCall& operator=(ToxGroupCall&& other) noexcept;

    QMap<int, quint32> peers;

    // If you add something here, don't forget to override the ctors and move operators!
};

#endif // TOXCALL_H
