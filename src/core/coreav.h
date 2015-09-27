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

#include <QHash>
#include <QThread>
#include <memory>
#include <map>
#include <tox/toxav.h>

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/al.h>
 #include <AL/alc.h>
#endif

#include "src/core/indexedlist.h"

#ifdef QTOX_FILTER_AUDIO
class AudioFilterer;
#endif

class QTimer;
class CoreVideoSource;
class CameraSource;
class VideoSource;
class VideoFrame;
class CoreAV;
struct vpx_image;

struct ToxCall
{
    ToxCall() = default;
    ToxCall(uint32_t FriendNum, bool VideoEnabled, CoreAV& av);
    ToxCall(const ToxCall& other) = delete;
    ToxCall(ToxCall&& other) noexcept;
    ~ToxCall();

    inline operator int() {return friendNum;}
    const ToxCall& operator=(const ToxCall& other) = delete;
    const ToxCall& operator=(ToxCall&& other) noexcept;

    QTimer* sendAudioTimer;
    uint32_t friendNum;
    bool muteMic;
    bool muteVol;
    bool videoEnabled; ///< True if our user asked for a video call, sending and recving
    ALuint alSource;
    CoreVideoSource* videoSource;
    TOXAV_FRIEND_CALL_STATE state; ///< State of the peer (not ours!)
#ifdef QTOX_FILTER_AUDIO
    AudioFilterer* filterer;
#endif
};

struct ToxGroupCall
{
    QTimer *sendAudioTimer;
    int groupId;
    bool active = false;
    bool muteMic;
    bool muteVol;
    QHash<int, ALuint> alSources;
};

class CoreAV : public QThread
{
    Q_OBJECT

public:
    CoreAV(Tox* tox);
    ~CoreAV();

    const ToxAV* getToxAv() const;

    void process();

    bool anyActiveCalls(); ///< true is any calls are currently active (note: a call about to start is not yet active)
    void prepareCall(uint32_t friendId, ToxAV *toxav, bool videoEnabled);
    void cleanupCall(uint32_t friendId);
    void playCallAudio(void *toxav, int32_t callId, const int16_t *data,
                              uint16_t samples, void *user_data); // Callback
    void sendCallAudio(uint32_t friendId);
    void playAudioBuffer(ALuint alSource, const int16_t *data, int samples,
                                unsigned channels, int sampleRate);
    void playCallVideo(void *toxav, int32_t callId, const vpx_image* img, void *user_data);
    void sendCallVideo(uint32_t friendId, std::shared_ptr<VideoFrame> frame);
    void sendGroupCallAudio(int groupId, ToxAV* toxav);

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

signals:
    void avInvite(uint32_t friendId, bool video);
    void avStart(uint32_t friendId, bool video);
    void avCancel(uint32_t friendId);
    void avEnd(uint32_t friendId);
    void avRinging(uint32_t friendId, bool video);
    void avStarting(uint32_t friendId, bool video);
    void avEnding(uint32_t friendId);
    void avRequestTimeout(uint32_t friendId);
    void avPeerTimeout(uint32_t friendId);
    void avMediaChange(uint32_t friendId, bool videoEnabled);
    void avCallFailed(uint32_t friendId);
    void avRejected(uint32_t friendId);

    void videoFrameReceived(vpx_image* frame);

private:
    static void callCallback(ToxAV *toxAV, uint32_t friendNum, bool audio, bool video, void* self);
    static void stateCallback(ToxAV *toxAV, uint32_t friendNum, uint32_t state, void* self);
    static void audioBitrateCallback(ToxAV *toxAV, uint32_t friendNum, bool stable, uint32_t rate, void* self);
    static void videoBitrateCallback(ToxAV *toxAV, uint32_t friendNum, bool stable, uint32_t rate, void* self);
    static void audioFrameCallback(ToxAV *toxAV, uint32_t friendNum, const int16_t *pcm, size_t sampleCount,
                                  uint8_t channels, uint32_t samplingRate, void* self);
    static void videoFrameCallback(ToxAV *toxAV, uint32_t friendNum, uint16_t w, uint16_t h,
                                   const uint8_t *y, const uint8_t *u, const uint8_t *v,
                                   int32_t ystride, int32_t ustride, int32_t vstride, void* self);

private:
    static constexpr uint32_t AUDIO_DEFAULT_BITRATE = 64; ///< In kb/s. More than enough for Opus.
    static constexpr uint32_t VIDEO_DEFAULT_BITRATE = 384; ///< Picked at random by fair dice roll.

private:
    ToxAV* toxav;
    static IndexedList<ToxCall> calls;
    static QHash<int, ToxGroupCall> groupCalls; // Maps group IDs to ToxGroupCalls

    friend class Audio;
    friend class Core;
};

#endif // COREAV_H
