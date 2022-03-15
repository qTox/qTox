/*
    Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "src/core/toxcall.h"
#include "util/compatiblerecursivemutex.h"

#include <QObject>
#include <QMutex>
#include <QReadWriteLock>
#include <atomic>
#include <memory>
#include <tox/toxav.h>

class Friend;
class Group;
class IAudioControl;
class IAudioSettings;
class IGroupSettings;
class QThread;
class QTimer;
class CoreVideoSource;
class CameraSource;
class VideoSource;
class VideoFrame;
class Core;
struct vpx_image;

class CoreAV : public QObject
{
    Q_OBJECT

public:
    using CoreAVPtr = std::unique_ptr<CoreAV>;
    static CoreAVPtr makeCoreAV(Tox* core, CompatibleRecursiveMutex& toxCoreLock,
                                IAudioSettings& audioSettings, IGroupSettings& groupSettings,
                                CameraSource& cameraSource);

    void setAudio(IAudioControl& newAudio);
    IAudioControl* getAudio();

    ~CoreAV();

    bool isCallStarted(const Friend* f) const;
    bool isCallStarted(const Group* g) const;
    bool isCallActive(const Friend* f) const;
    bool isCallActive(const Group* g) const;
    bool isCallVideoEnabled(const Friend* f) const;
    bool sendCallAudio(uint32_t callId, const int16_t* pcm, size_t samples, uint8_t chans,
                       uint32_t rate) const;
    void sendCallVideo(uint32_t callId, std::shared_ptr<VideoFrame> frame);
    bool sendGroupCallAudio(int groupNum, const int16_t* pcm, size_t samples, uint8_t chans,
                            uint32_t rate) const;

    VideoSource* getVideoSourceFromCall(int friendNum) const;
    void sendNoVideo();

    void joinGroupCall(const Group& group);
    void leaveGroupCall(int groupNum);
    void muteCallInput(const Group* g, bool mute);
    void muteCallOutput(const Group* g, bool mute);
    bool isGroupCallInputMuted(const Group* g) const;
    bool isGroupCallOutputMuted(const Group* g) const;

    bool isCallInputMuted(const Friend* f) const;
    bool isCallOutputMuted(const Friend* f) const;
    void toggleMuteCallInput(const Friend* f);
    void toggleMuteCallOutput(const Friend* f);
    static void groupCallCallback(void* tox, uint32_t group, uint32_t peer, const int16_t* data,
                                  unsigned samples, uint8_t channels, uint32_t sample_rate,
                                  void* core);
    void invalidateGroupCallPeerSource(const Group& group, ToxPk peerPk);

public slots:
    bool startCall(uint32_t friendNum, bool video);
    bool answerCall(uint32_t friendNum, bool video);
    bool cancelCall(uint32_t friendNum);
    void timeoutCall(uint32_t friendNum);
    void start();

signals:
    void avInvite(uint32_t friendId, bool video);
    void avStart(uint32_t friendId, bool video);
    void avEnd(uint32_t friendId, bool error = false);

private slots:
    static void callCallback(ToxAV* toxAV, uint32_t friendNum, bool audio, bool video, void* self);
    static void stateCallback(ToxAV* toxAV, uint32_t friendNum, uint32_t state, void* self);
    static void bitrateCallback(ToxAV* toxAV, uint32_t friendNum, uint32_t arate, uint32_t vrate,
                                void* self);
    static void audioBitrateCallback(ToxAV* toxAV, uint32_t friendNum, uint32_t rate, void* self);
    static void videoBitrateCallback(ToxAV* toxAV, uint32_t friendNum, uint32_t rate, void* self);

private:
    struct ToxAVDeleter
    {
        void operator()(ToxAV* tox)
        {
            toxav_kill(tox);
        }
    };

    CoreAV(std::unique_ptr<ToxAV, ToxAVDeleter> toxav_, CompatibleRecursiveMutex &toxCoreLock,
           IAudioSettings& audioSettings_, IGroupSettings& groupSettings_, CameraSource& cameraSource);
    void connectCallbacks();

    void process();
    static void audioFrameCallback(ToxAV* toxAV, uint32_t friendNum, const int16_t* pcm,
                                   size_t sampleCount, uint8_t channels, uint32_t samplingRate,
                                   void* self);
    static void videoFrameCallback(ToxAV* toxAV, uint32_t friendNum, uint16_t w, uint16_t h,
                                   const uint8_t* y, const uint8_t* u, const uint8_t* v,
                                   int32_t ystride, int32_t ustride, int32_t vstride, void* self);

private:
    static constexpr uint32_t VIDEO_DEFAULT_BITRATE = 2500;

private:
    // atomic because potentially accessed by different threads
    std::atomic<IAudioControl*> audio;
    std::unique_ptr<ToxAV, ToxAVDeleter> toxav;
    std::unique_ptr<QThread> coreavThread;
    QTimer* iterateTimer = nullptr;
    using ToxFriendCallPtr = std::unique_ptr<ToxFriendCall>;
    /**
     * @brief Maps friend IDs to ToxFriendCall.
     * @note Need to use STL container here, because Qt containers need a copy constructor.
     */
    std::map<uint32_t, ToxFriendCallPtr> calls;


    using ToxGroupCallPtr = std::unique_ptr<ToxGroupCall>;
    /**
     * @brief Maps group IDs to ToxGroupCalls.
     * @note Need to use STL container here, because Qt containers need a copy constructor.
     */
    std::map<int, ToxGroupCallPtr> groupCalls;

    // protect 'calls' and 'groupCalls'
    mutable QReadWriteLock callsLock{QReadWriteLock::Recursive};

    /**
     * @brief needed to synchronize with the Core thread, some toxav_* functions
     *        must not execute at the same time as tox_iterate()
     * @note This must be a recursive mutex as we're going to lock it in callbacks
     */
    CompatibleRecursiveMutex& coreLock;

    IAudioSettings& audioSettings;
    IGroupSettings& groupSettings;
    CameraSource& cameraSource;
};
