/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is free software: you can redistribute it and/or modify
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

#include "core.h"
#include "coreav.h"
#include "src/video/camerasource.h"
#include "src/video/corevideosource.h"
#include "src/video/videoframe.h"
#include "src/audio/audio.h"
#ifdef QTOX_FILTER_AUDIO
#include "src/audio/audiofilterer.h"
#endif
#include "src/persistence/settings.h"
#include <assert.h>
#include <QDebug>
#include <QTimer>

IndexedList<ToxCall> CoreAV::calls;
QHash<int, ToxGroupCall> CoreAV::groupCalls;

ToxCall::ToxCall(uint32_t FriendNum, bool VideoEnabled, CoreAV& av)
    : sendAudioTimer{new QTimer}, friendNum{FriendNum},
      ringing{true}, muteMic{false}, muteVol{false},
      videoEnabled{VideoEnabled},
      alSource{0}, videoSource{nullptr},
      state{static_cast<TOXAV_FRIEND_CALL_STATE>(0)}
{
    Audio::getInstance().subscribeInput();
    sendAudioTimer->setInterval(5);
    sendAudioTimer->setSingleShot(true);
    QObject::connect(sendAudioTimer, &QTimer::timeout, [FriendNum,&av](){av.sendCallAudio(FriendNum);});
    sendAudioTimer->start();

    if (videoEnabled)
    {
        videoSource = new CoreVideoSource;
        CameraSource& source = CameraSource::getInstance();
        source.subscribe();
        QObject::connect(&source, &VideoSource::frameAvailable,
                [=,&av](std::shared_ptr<VideoFrame> frame){av.sendCallVideo(friendNum,frame);});
    }


#ifdef QTOX_FILTER_AUDIO
    if (Settings::getInstance().getFilterAudio())
    {
        filterer = new AudioFilterer();
        filterer->startFilter(48000);
    }
    else
    {
        filterer = nullptr;
    }
#endif
}

ToxCall::ToxCall(ToxCall&& other)
    : sendAudioTimer{other.sendAudioTimer}, friendNum{other.friendNum},
      ringing{other.ringing}, muteMic{other.muteMic}, muteVol{other.muteVol},
      videoEnabled{other.videoEnabled},
      alSource{other.alSource}, videoSource{other.videoSource},
      state{other.state}
{
    other.sendAudioTimer = nullptr;
    other.friendNum = std::numeric_limits<decltype(friendNum)>::max();
    other.alSource = 0;
    other.videoSource = nullptr;

#ifdef QTOX_FILTER_AUDIO
    filterer = other.filterer;
    other.filterer = nullptr;
#endif
}

ToxCall::~ToxCall()
{
    if (sendAudioTimer)
    {
        QObject::disconnect(sendAudioTimer, nullptr, nullptr, nullptr);
        sendAudioTimer->stop();
        Audio::getInstance().unsubscribeInput();
    }
    if (videoEnabled)
    {
        CameraSource::getInstance().unsubscribe();
        if (videoSource)
        {
            videoSource->setDeleteOnClose(true);
            videoSource = nullptr;
        }
    }
}

const ToxCall& ToxCall::operator=(ToxCall&& other)
{
    sendAudioTimer = other.sendAudioTimer;
    other.sendAudioTimer = nullptr;
    friendNum = other.friendNum;
    other.friendNum = std::numeric_limits<decltype(friendNum)>::max();
    ringing = other.ringing;
    muteMic = other.muteMic;
    muteVol = other.muteVol;
    videoEnabled = other.videoEnabled;
    alSource = other.alSource;
    other.alSource = 0;
    videoSource = other.videoSource;
    other.videoSource = nullptr;
    state = other.state;

    #ifdef QTOX_FILTER_AUDIO
        filterer = other.filterer;
        other.filterer = nullptr;
    #endif

    return *this;
}

CoreAV::CoreAV(Tox *tox)
{
    toxav = toxav_new(tox, nullptr);

    toxav_callback_call(toxav, CoreAV::callCallback, this);
    toxav_callback_call_state(toxav, CoreAV::stateCallback, this);
    toxav_callback_audio_bit_rate_status(toxav, CoreAV::audioBitrateCallback, this);
    toxav_callback_video_bit_rate_status(toxav, CoreAV::videoBitrateCallback, this);
    toxav_callback_audio_receive_frame(toxav, CoreAV::audioFrameCallback, this);
    toxav_callback_video_receive_frame(toxav, CoreAV::videoFrameCallback, this);
}

CoreAV::~CoreAV()
{
    for (const ToxCall& call : calls)
        cancelCall(call.friendNum);
    toxav_kill(toxav);
}

const ToxAV *CoreAV::getToxAv() const
{
    return toxav;
}

void CoreAV::process()
{
    toxav_iterate(toxav);
}

bool CoreAV::anyActiveCalls()
{
    return !calls.isEmpty();
}

void CoreAV::answerCall(uint32_t friendNum)
{
    qDebug() << QString("answering call %1").arg(friendNum);
    assert(calls.contains(friendNum));
    TOXAV_ERR_ANSWER err;
    if (toxav_answer(toxav, friendNum, AUDIO_DEFAULT_BITRATE, VIDEO_DEFAULT_BITRATE, &err))
    {
        calls[friendNum].ringing = false;
        emit avStart(friendNum, calls[friendNum].videoEnabled);
    }
    else
    {
        qWarning() << "Failed to answer call with error"<<err;
        calls.remove(friendNum);
        emit avCallFailed(friendNum);
    }
}

void CoreAV::startCall(uint32_t friendId, bool video)
{
    qWarning() << "START CALL CALLED";
    assert(!calls.contains(friendId));
    uint32_t videoBitrate = video ? VIDEO_DEFAULT_BITRATE : 0;
    if (!toxav_call(toxav, friendId, AUDIO_DEFAULT_BITRATE, videoBitrate, nullptr))
    {
        emit avCallFailed(friendId);
        return;
    }

    calls.insert({friendId, video, *this});
    emit avRinging(friendId, video);
}

void CoreAV::cancelCall(uint32_t friendId)
{
    qDebug() << QString("Cancelling call with %1").arg(friendId);
    toxav_call_control(toxav, friendId, TOXAV_CALL_CONTROL_CANCEL, nullptr);
    calls.remove(friendId);
}

void CoreAV::playCallAudio(void* toxav, int32_t callId, const int16_t *data, uint16_t samples, void *user_data)
{
    Q_UNUSED(user_data);

    if (!calls.contains(callId) || calls[callId].muteVol)
        return;

    if (!calls[callId].alSource)
        alGenSources(1, &calls[callId].alSource);

    //ToxAvCSettings dest;
    //if (toxav_get_peer_csettings((ToxAV*)toxav, callId, 0, &dest) == 0)
    //    playAudioBuffer(calls[callId].alSource, data, samples, dest.audio_channels, dest.audio_sample_rate);
}

void CoreAV::sendCallAudio(uint32_t callId)
{
    if (!calls.contains(callId))
        return;

    ToxCall& call = calls[callId];

    if (call.muteMic || call.ringing
            || !(call.state & TOXAV_FRIEND_CALL_STATE_ACCEPTING_A)
            || !Audio::getInstance().isInputReady())
    {
        call.sendAudioTimer->start();
        return;
    }

    int16_t buf[AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS] = {0};
    if (Audio::getInstance().tryCaptureSamples(buf, AUDIO_FRAME_SAMPLE_COUNT))
    {
#ifdef QTOX_FILTER_AUDIO
        if (Settings::getInstance().getFilterAudio())
        {
            if (!call.filterer)
            {
                call.filterer = new AudioFilterer();
                call.filterer->startFilter(AUDIO_SAMPLE_RATE);
            }
            // is a null op #ifndef ALC_LOOPBACK_CAPTURE_SAMPLES
            Audio::getEchoesToFilter(call.filterer, AUDIO_FRAME_SAMPLE_COUNT);

            call.filterer->filterAudio(buf, AUDIO_FRAME_SAMPLE_COUNT);
        }
        else if (call.filterer)
        {
            delete call.filterer;
            call.filterer = nullptr;
        }
#endif

        if (!toxav_audio_send_frame(toxav, callId, buf, AUDIO_FRAME_SAMPLE_COUNT,
                                    AUDIO_CHANNELS, AUDIO_SAMPLE_RATE, nullptr))
            qDebug() << "toxav_audio_send_frame error";
    }

    call.sendAudioTimer->start();
}

void CoreAV::playCallVideo(void*, int32_t callId, const vpx_image *img, void *user_data)
{
    Q_UNUSED(user_data);

    if (!calls.contains(callId) || !calls[callId].videoEnabled)
        return;

    calls[callId].videoSource->pushFrame(img);
}

void CoreAV::sendCallVideo(uint32_t callId, std::shared_ptr<VideoFrame> vframe)
{
    if (!calls.contains(callId) || !calls[callId].videoEnabled
            || !(calls[callId].state & TOXAV_FRIEND_CALL_STATE_ACCEPTING_V))
        return;

    // This frame shares vframe's buffers, we don't call vpx_img_free but just delete it
    vpx_image* frame = vframe->toVpxImage();
    if (frame->fmt == VPX_IMG_FMT_NONE)
    {
        qWarning() << "Invalid frame";
        delete frame;
        return;
    }

#if 0
    int result;
    if ((result = toxav_prepare_video_frame(toxav, callId, videobuf, videobufsize, frame)) < 0)
    {
        qDebug() << QString("toxav_prepare_video_frame: error %1").arg(result);
        delete frame;
        return;
    }

    if ((result = toxav_send_video(toxav, callId, (uint8_t*)videobuf, result)) < 0)
        qDebug() << QString("toxav_send_video error: %1").arg(result);
#endif

    delete frame;
}

void CoreAV::micMuteToggle(uint32_t callId)
{
    if (calls.contains(callId))
        calls[callId].muteMic = !calls[callId].muteMic;
}

void CoreAV::volMuteToggle(uint32_t callId)
{
    if (calls.contains(callId))
        calls[callId].muteVol = !calls[callId].muteVol;
}

VideoSource *CoreAV::getVideoSourceFromCall(int friendNum)
{
    assert(calls.contains(friendNum));
    return calls[friendNum].videoSource;
}

void CoreAV::joinGroupCall(int groupId)
{
    qDebug() << QString("Joining group call %1").arg(groupId);
    groupCalls[groupId].groupId = groupId;
    groupCalls[groupId].muteMic = false;
    groupCalls[groupId].muteVol = false;
    // the following three lines are also now redundant from startCall, but are
    // necessary there for outbound and here for inbound

    // Audio
    Audio::getInstance().subscribeInput();

    // Go
    groupCalls[groupId].sendAudioTimer = new QTimer();
    groupCalls[groupId].active = true;
    groupCalls[groupId].sendAudioTimer->setInterval(5);
    groupCalls[groupId].sendAudioTimer->setSingleShot(true);
    connect(groupCalls[groupId].sendAudioTimer, &QTimer::timeout, [=](){sendGroupCallAudio(groupId,toxav);});
    groupCalls[groupId].sendAudioTimer->start();
}

void CoreAV::leaveGroupCall(int groupId)
{
    qDebug() << QString("Leaving group call %1").arg(groupId);
    groupCalls[groupId].active = false;
    disconnect(groupCalls[groupId].sendAudioTimer,0,0,0);
    groupCalls[groupId].sendAudioTimer->stop();
    for (ALuint source : groupCalls[groupId].alSources)
        alDeleteSources(1, &source);
    groupCalls[groupId].alSources.clear();
    Audio::getInstance().unsubscribeInput();
    delete groupCalls[groupId].sendAudioTimer;
}

void CoreAV::sendGroupCallAudio(int groupId, ToxAV *toxav)
{
    if (!groupCalls[groupId].active)
        return;

    if (groupCalls[groupId].muteMic || !Audio::getInstance().isInputReady())
    {
        groupCalls[groupId].sendAudioTimer->start();
        return;
    }

#if 0
    const int framesize = (groupCalls[groupId].codecSettings.audio_frame_duration * groupCalls[groupId].codecSettings.audio_sample_rate) / 1000 * av_DefaultSettings.audio_channels;
    const int bufsize = framesize * 2 * av_DefaultSettings.audio_channels;
    uint8_t buf[bufsize];

    if (Audio::getInstance().tryCaptureSamples(buf, framesize))
    {
        if (toxav_group_send_audio(toxav_get_tox(toxav), groupId, (int16_t*)buf,
                framesize, av_DefaultSettings.audio_channels, av_DefaultSettings.audio_sample_rate) < 0)
        {
            qDebug() << "toxav_group_send_audio error";
            groupCalls[groupId].sendAudioTimer->start();
            return;
        }
    }
#endif
    groupCalls[groupId].sendAudioTimer->start();
}

void CoreAV::disableGroupCallMic(int groupId)
{
    groupCalls[groupId].muteMic = true;
}

void CoreAV::disableGroupCallVol(int groupId)
{
    groupCalls[groupId].muteVol = true;
}

void CoreAV::enableGroupCallMic(int groupId)
{
    groupCalls[groupId].muteMic = false;
}

void CoreAV::enableGroupCallVol(int groupId)
{
    groupCalls[groupId].muteVol = false;
}

bool CoreAV::isGroupCallMicEnabled(int groupId) const
{
    return !groupCalls[groupId].muteMic;
}

bool CoreAV::isGroupCallVolEnabled(int groupId) const
{
    return !groupCalls[groupId].muteVol;
}

bool CoreAV::isGroupAvEnabled(int groupId) const
{
    return tox_group_get_type(Core::getInstance()->tox, groupId) == TOX_GROUPCHAT_TYPE_AV;
}

void CoreAV::resetCallSources()
{
    for (ToxGroupCall& call : groupCalls)
    {
        for (ALuint source : call.alSources)
            alDeleteSources(1, &source);
        call.alSources.clear();
    }

    for (ToxCall& call : calls)
    {
        if (call.alSource)
        {
            ALuint tmp = call.alSource;
            call.alSource = 0;
            alDeleteSources(1, &tmp);

            alGenSources(1, &call.alSource);
        }
    }
}

void CoreAV::callCallback(ToxAV*, uint32_t friendNum, bool audio, bool video, void *_self)
{
    qWarning() << "RECEIVED CALL";
    CoreAV* self = static_cast<CoreAV*>(_self);
    const auto& callIt = calls.insert({friendNum, video, *self});

    // We don't get a state callback when answering, so fill the state ourselves in advance
    int state = 0;
    if (audio)
        state |= TOXAV_FRIEND_CALL_STATE_SENDING_A | TOXAV_FRIEND_CALL_STATE_ACCEPTING_A;
    if (video)
        state |= TOXAV_FRIEND_CALL_STATE_SENDING_V | TOXAV_FRIEND_CALL_STATE_ACCEPTING_V;
    callIt->state = static_cast<TOXAV_FRIEND_CALL_STATE>(state);

    emit reinterpret_cast<CoreAV*>(self)->avInvite(friendNum, video);
}

void CoreAV::stateCallback(ToxAV *toxAV, uint32_t friendNum, uint32_t state, void *_self)
{
    qWarning() << "STATE IS "<<state;
    CoreAV* self = static_cast<CoreAV*>(_self);

    assert(self->calls.contains(friendNum));
    ToxCall& call = self->calls[friendNum];

    if (state & TOXAV_FRIEND_CALL_STATE_ERROR)
    {
        qWarning() << "Call with friend"<<friendNum<<"died of unnatural causes";
        calls.remove(friendNum);
        emit self->avCallFailed(friendNum);
    }
    else if (state & TOXAV_FRIEND_CALL_STATE_FINISHED)
    {
        calls.remove(friendNum);
        emit self->avEnd(friendNum);
    }
    else
    {
        // If our state was null, we started the call and still ringing
        if (!call.state && state)
        {
            call.ringing = false;
            emit self->avStart(friendNum, call.videoEnabled);
        }

        call.state = static_cast<TOXAV_FRIEND_CALL_STATE>(state);
    }
}

void CoreAV::audioBitrateCallback(ToxAV *toxAV, uint32_t friendNum, bool stable, uint32_t rate, void *self)
{
    qWarning() << "AUDIO BITRATE IS "<<rate<<" STABILITY:"<<stable;
}

void CoreAV::videoBitrateCallback(ToxAV *toxAV, uint32_t friendNum, bool stable, uint32_t rate, void *self)
{
    qWarning() << "AUDIO BITRATE IS "<<rate<<" STABILITY:"<<stable;
}

void CoreAV::audioFrameCallback(ToxAV *toxAV, uint32_t friendNum, const int16_t *pcm,
                                size_t sampleCount, uint8_t channels, uint32_t samplingRate, void *_self)
{
    qWarning() << "AUDIO FRAME"<<sampleCount<<"SAMPLES AT"<<samplingRate<<"kHz"<<channels<<"CHANNELS";
    CoreAV* self = static_cast<CoreAV*>(_self);
    if (!self->calls.contains(friendNum))
        return;

    ToxCall& call = self->calls[friendNum];

    if (call.muteVol)
        return;

    if (!call.alSource)
        alGenSources(1, &call.alSource);

    Audio::playAudioBuffer(call.alSource, pcm, sampleCount, channels, samplingRate);
}

void CoreAV::videoFrameCallback(ToxAV *toxAV, uint32_t friendNum, uint16_t w, uint16_t h, const uint8_t *y, const uint8_t *u, const uint8_t *v, int32_t ystride, int32_t ustride, int32_t vstride, void *self)
{
    qWarning() << "VIDEO FRAME";
}
