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
    void prepareCall(uint32_t friendId, ToxAV *toxav, bool videoEnabled);
    void cleanupCall(uint32_t friendId);
    bool sendCallAudio(uint32_t friendId); ///< Returns false only on error, but not if there's nothing to send
    void playAudioBuffer(ALuint alSource, const int16_t *data, int samples,
                                unsigned channels, int sampleRate);
    void sendCallVideo(uint32_t friendId, std::shared_ptr<VideoFrame> frame);
    bool sendGroupCallAudio(int groupId);

    VideoSource* getVideoSourceFromCall(int callNumber); ///< Get a call's video source
    void resetCallSources(); ///< Forces to regenerate each call's audio sources

    void joinGroupCall(int groupId); ///< Starts a call in an existing AV groupchat. Call from the GUI thread.
    void leaveGroupCall(int groupId); ///< Will not leave the group, just stop the call. Call from the GUI thread.
    void disableGroupCallMic(int groupId);
    void disableGroupCallVol(int groupId);
    void enableGroupCallMic(int groupId);
    void enableGroupCallVol(int groupId);
    bool isGroupCallMicEnabled(int groupId) const;
    bool isGroupCallVolEnabled(int groupId) const;
    bool isGroupAvEnabled(int groupId) const; ///< True for AV groups, false for text-only groups

    void startCall(uint32_t friendId, bool video=false);
    void answerCall(uint32_t friendId);
    void cancelCall(uint32_t friendId);

    void micMuteToggle(uint32_t friendId);
    void volMuteToggle(uint32_t friendId);

public slots:
    void start();
    void stop();

signals:
    void avInvite(uint32_t friendId, bool video);
    void avStart(uint32_t friendId, bool video);
    void avCancel(uint32_t friendId);
    void avEnd(uint32_t friendId);
    void avRinging(uint32_t friendId, bool video);
    void avCallFailed(uint32_t friendId);

    void videoFrameReceived(vpx_image* frame);

private:
    void process();
    static void callCallback(ToxAV *toxAV, uint32_t friendNum, bool audio, bool video, void* self);
    static void stateCallback(ToxAV *toxAV, uint32_t friendNum, uint32_t state, void* self);
    static void audioBitrateCallback(ToxAV *toxAV, uint32_t friendNum, bool stable, uint32_t rate, void* self);
    static void videoBitrateCallback(ToxAV *toxAV, uint32_t friendNum, bool stable, uint32_t rate, void* self);
    static void audioFrameCallback(ToxAV *toxAV, uint32_t friendNum, const int16_t *pcm, size_t sampleCount,
                                  uint8_t channels, uint32_t samplingRate, void* self);
    static void videoFrameCallback(ToxAV *toxAV, uint32_t friendNum, uint16_t w, uint16_t h,
                                   const uint8_t *y, const uint8_t *u, const uint8_t *v,
                                   int32_t ystride, int32_t ustride, int32_t vstride, void* self);
    /// Intercepts a function call and moves it to another thread
    /// Useful to move callbacks from the toxcore thread to our thread
    template <class... Args> void asyncTransplantThunk(void(*fun)(Args...), Args... args);

private:
    static constexpr uint32_t AUDIO_DEFAULT_BITRATE = 64; ///< In kb/s. More than enough for Opus.
    static constexpr uint32_t VIDEO_DEFAULT_BITRATE = 3000; ///< Picked at random by fair dice roll.

private:
    ToxAV* toxav;
    std::unique_ptr<QThread> coreavThread;
    std::unique_ptr<QTimer> iterateTimer;
    static IndexedList<ToxFriendCall> calls;
    static IndexedList<ToxGroupCall> groupCalls; // Maps group IDs to ToxGroupCalls

    friend class Audio;
};

#endif // COREAV_H
