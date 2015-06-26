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
#include <tox/toxav.h>

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/al.h>
 #include <AL/alc.h>
#endif

#ifdef QTOX_FILTER_AUDIO
class AudioFilterer;
#endif

class QTimer;
class CoreVideoSource;
class CameraSource;
class VideoSource;
class VideoFrame;
struct vpx_image;

struct ToxCall
{
    QTimer *sendAudioTimer;
    int32_t callId;
    uint32_t friendId;
    bool videoEnabled;
    bool active;
    bool muteMic;
    bool muteVol;
    ALuint alSource;
    CoreVideoSource* videoSource;
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
    CoreAV() = default;
    ~CoreAV();

    static bool anyActiveCalls(); ///< true is any calls are currently active (note: a call about to start is not yet active)
    static void prepareCall(uint32_t friendId, int callId, ToxAV *toxav, bool videoEnabled);
    static void cleanupCall(int callId);
    static void playCallAudio(void *toxav, int32_t callId, const int16_t *data,
                              uint16_t samples, void *user_data); // Callback
    static void sendCallAudio(int callId, ToxAV* toxav);
    static void playAudioBuffer(ALuint alSource, const int16_t *data, int samples,
                                unsigned channels, int sampleRate);
    static void playCallVideo(void *toxav, int32_t callId, const vpx_image* img, void *user_data);
    static void sendCallVideo(int callId, ToxAV* toxav, std::shared_ptr<VideoFrame> frame);
    static void sendGroupCallAudio(int groupId, ToxAV* toxav);

    static VideoSource* getVideoSourceFromCall(int callNumber); ///< Get a call's video source
    static void resetCallSources(); ///< Forces to regenerate each call's audio sources

    static void joinGroupCall(int groupId); ///< Starts a call in an existing AV groupchat. Call from the GUI thread.
    static void leaveGroupCall(int groupId); ///< Will not leave the group, just stop the call. Call from the GUI thread.
    static void disableGroupCallMic(int groupId);
    static void disableGroupCallVol(int groupId);
    static void enableGroupCallMic(int groupId);
    static void enableGroupCallVol(int groupId);
    static bool isGroupCallMicEnabled(int groupId);
    static bool isGroupCallVolEnabled(int groupId);
    static bool isGroupAvEnabled(int groupId); ///< True for AV groups, false for text-only groups

public slots:
    static void answerCall(int callId);
    static void rejectCall(int callId);
    static void hangupCall(int callId);
    static void startCall(uint32_t friendId, bool video=false);
    static void cancelCall(int callId, uint32_t friendId);

    static void micMuteToggle(int callId);
    static void volMuteToggle(int callId);

private:
    static QVector<ToxCall> calls;
#ifdef QTOX_FILTER_AUDIO
    static QVector<AudioFilterer*> filterer;
#endif
    static QHash<int, ToxGroupCall> groupCalls; // Maps group IDs to ToxGroupCalls

    friend class Audio;
    friend class Core;
};

#endif // COREAV_H
