/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2015 by The qTox Project

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

#ifndef COREAV_H
#define COREAV_H

#include <QObject>
#include <memory>
#include <atomic>
#include <tox/toxav.h>

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/al.h>
 #include <AL/alc.h>
#endif

#include "src/core/toxcall.h"

#ifdef QTOX_FILTER_AUDIO
class AudioFilterer;
#endif

class QTimer;
class QThread;
class CoreVideoSource;
class CameraSource;
class VideoSource;
class VideoFrame;
class CoreAV;
struct vpx_image;

class CoreAV : public QObject
{
    Q_OBJECT

public:
    CoreAV(Tox* tox);
    ~CoreAV();

    const ToxAV* getToxAv() const;

    bool anyActiveCalls(); ///< true is any calls are currently active (note: a call about to start is not yet active)
    bool isCallVideoEnabled(uint32_t friendNum);
    bool sendCallAudio(uint32_t friendNum); ///< Returns false only on error, but not if there's nothing to send
    void sendCallVideo(uint32_t friendNum, std::shared_ptr<VideoFrame> frame);
    bool sendGroupCallAudio(int groupNum);

    VideoSource* getVideoSourceFromCall(int callNumber); ///< Get a call's video source
    void resetCallSources(); ///< Forces to regenerate each call's audio sources
    void sendNoVideo(); ///< Signal to all peers that we're not sending video anymore. The next frame sent cancels this.

    void joinGroupCall(int groupNum); ///< Starts a call in an existing AV groupchat. Call from the GUI thread.
    void leaveGroupCall(int groupNum); ///< Will not leave the group, just stop the call. Call from the GUI thread.
    void disableGroupCallMic(int groupNum);
    void disableGroupCallVol(int groupNum);
    void enableGroupCallMic(int groupNum);
    void enableGroupCallVol(int groupNum);
    bool isGroupCallMicEnabled(int groupNum) const;
    bool isGroupCallVolEnabled(int groupNum) const;
    bool isGroupAvEnabled(int groupNum) const; ///< True for AV groups, false for text-only groups

    void micMuteToggle(uint32_t friendNum);
    void volMuteToggle(uint32_t friendNum);

public slots:
    bool startCall(uint32_t friendNum, bool video=false);
    bool answerCall(uint32_t friendNum);
    bool cancelCall(uint32_t friendNum);
    void timeoutCall(uint32_t friendNum);
    /// Starts the CoreAV main loop that calls toxav's main loop
    void start();
    /// Stops the main loop
    void stop();

signals:
    void avInvite(uint32_t friendId, bool video); ///< Sent when a friend calls us
    void avStart(uint32_t friendId, bool video); ///< Sent when a call we initiated has started
    void avEnd(uint32_t friendId); ///< Sent when a call was ended by the peer

private slots:
    static void callCallback(ToxAV *toxAV, uint32_t friendNum, bool audio, bool video, void* self);
    static void stateCallback(ToxAV *, uint32_t friendNum, uint32_t state, void* self);
    static void bitrateCallback(ToxAV *toxAV, uint32_t friendNum, uint32_t arate, uint32_t vrate, void* self);
    void killTimerFromThread(); ///< Calls itself blocking queued on the coreav thread

private:
    void process();
    static void audioFrameCallback(ToxAV *toxAV, uint32_t friendNum, const int16_t *pcm, size_t sampleCount,
                                  uint8_t channels, uint32_t samplingRate, void* self);
    static void videoFrameCallback(ToxAV *toxAV, uint32_t friendNum, uint16_t w, uint16_t h,
                                   const uint8_t *y, const uint8_t *u, const uint8_t *v,
                                   int32_t ystride, int32_t ustride, int32_t vstride, void* self);

private:
    static constexpr uint32_t AUDIO_DEFAULT_BITRATE = 64; ///< In kb/s. More than enough for Opus.
    static constexpr uint32_t VIDEO_DEFAULT_BITRATE = 6144; ///< Picked at random by fair dice roll.

private:
    ToxAV* toxav;
    std::unique_ptr<QThread> coreavThread;
    std::unique_ptr<QTimer> iterateTimer;
    static IndexedList<ToxFriendCall> calls;
    static IndexedList<ToxGroupCall> groupCalls; // Maps group IDs to ToxGroupCalls
    /**
     * This flag is to be acquired before switching in a blocking way between the UI and CoreAV thread.
     * The CoreAV thread must have priority for the flag, other threads should back off or release it quickly.
     *
     * CoreAV needs to interface with three threads, the toxcore/Core thread that fires non-payload
     * toxav callbacks, the toxav/CoreAV thread that fires AV payload callbacks and manages
     * most of CoreAV's members, and the UI thread, which calls our [start/answer/cancel]Call functions
     * and which we call via signals.
     * When the UI calls us, we switch from the UI thread to the CoreAV thread to do the processing,
     * when toxcore fires a non-payload av callback, we do the processing in the CoreAV thread and then
     * switch to the UI thread to send it a signal. Both switches block both threads, so this would deadlock.
     */
    std::atomic_flag threadSwitchLock;

    friend class Audio;
};

#endif // COREAV_H
