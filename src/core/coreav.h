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
#include "src/core/toxcall.h"
#include <tox/toxav.h>

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
    explicit CoreAV(Tox* tox);
    ~CoreAV();

    const ToxAV* getToxAv() const;

    bool anyActiveCalls();
    bool isCallVideoEnabled(uint32_t friendNum);
    bool sendCallAudio(uint32_t friendNum, const int16_t *pcm, size_t samples, uint8_t chans, uint32_t rate);
    void sendCallVideo(uint32_t friendNum, std::shared_ptr<VideoFrame> frame);
    bool sendGroupCallAudio(int groupNum, const int16_t *pcm, size_t samples, uint8_t chans, uint32_t rate);

    VideoSource* getVideoSourceFromCall(int callNumber);
    void invalidateCallSources();
    void sendNoVideo();

    void joinGroupCall(int groupNum);
    void leaveGroupCall(int groupNum);
    void disableGroupCallMic(int groupNum);
    void disableGroupCallVol(int groupNum);
    void enableGroupCallMic(int groupNum);
    void enableGroupCallVol(int groupNum);
    bool isGroupCallMicEnabled(int groupNum) const;
    bool isGroupCallVolEnabled(int groupNum) const;
    bool isGroupAvEnabled(int groupNum) const;

    void micMuteToggle(uint32_t friendNum);
    void volMuteToggle(uint32_t friendNum);

    static void groupCallCallback(void* tox, int group, int peer,
                                  const int16_t* data, unsigned samples,
                                  uint8_t channels, unsigned sample_rate,
                                  void* core);

public slots:
    bool startCall(uint32_t friendNum, bool video=false);
    bool answerCall(uint32_t friendNum);
    bool cancelCall(uint32_t friendNum);
    void timeoutCall(uint32_t friendNum);
    void start();
    void stop();

signals:
    void avInvite(uint32_t friendId, bool video);
    void avStart(uint32_t friendId, bool video);
    void avEnd(uint32_t friendId);

private slots:
    static void callCallback(ToxAV *toxAV, uint32_t friendNum, bool audio, bool video, void* self);
    static void stateCallback(ToxAV *, uint32_t friendNum, uint32_t state, void* self);
    static void bitrateCallback(ToxAV *toxAV, uint32_t friendNum, uint32_t arate, uint32_t vrate, void* self);
    void killTimerFromThread();

private:
    void process();
    static void audioFrameCallback(ToxAV *toxAV, uint32_t friendNum, const int16_t *pcm, size_t sampleCount,
                                  uint8_t channels, uint32_t samplingRate, void* self);
    static void videoFrameCallback(ToxAV *toxAV, uint32_t friendNum, uint16_t w, uint16_t h,
                                   const uint8_t *y, const uint8_t *u, const uint8_t *v,
                                   int32_t ystride, int32_t ustride, int32_t vstride, void* self);

private:
    static constexpr uint32_t AUDIO_DEFAULT_BITRATE = 64;
    static constexpr uint32_t VIDEO_DEFAULT_BITRATE = 6144;

private:
    ToxAV* toxav;
    std::unique_ptr<QThread> coreavThread;
    std::unique_ptr<QTimer> iterateTimer;
    static IndexedList<ToxFriendCall> calls;
    static IndexedList<ToxGroupCall> groupCalls;
    std::atomic_flag threadSwitchLock;

    friend class Audio;
};

#endif // COREAV_H
