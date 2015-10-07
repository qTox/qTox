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
#include "src/audio/audio.h"
#include "src/persistence/settings.h"
#include "src/video/videoframe.h"
#include "src/video/corevideosource.h"
#include <assert.h>
#include <QDebug>

#ifdef QTOX_FILTER_AUDIO
#include "src/audio/audiofilterer.h"
#endif

IndexedList<ToxFriendCall> CoreAV::calls;
IndexedList<ToxGroupCall> CoreAV::groupCalls;

using namespace std;

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
    for (const ToxFriendCall& call : calls)
        cancelCall(call.callId);
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
        calls[friendNum].inactive = false;
        emit avStart(friendNum, calls[friendNum].videoEnabled);
    }
    else
    {
        qWarning() << "Failed to answer call with error"<<err;
        toxav_call_control(toxav, friendNum, TOXAV_CALL_CONTROL_CANCEL, nullptr);
        calls.remove(friendNum);
        emit avCallFailed(friendNum);
    }
}

void CoreAV::startCall(uint32_t friendId, bool video)
{
    qDebug() << QString("Starting call with %1").arg(friendId);
    if(calls.contains(friendId))
    {
        qWarning() << QString("Can't start call with %1, we're already in this call!").arg(friendId);
        emit avCallFailed(friendId);
        return;
    }

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
    if (!toxav_call_control(toxav, friendId, TOXAV_CALL_CONTROL_CANCEL, nullptr))
    {
        qWarning() << QString("Failed to cancel call with %1").arg(friendId);
        return;
    }
    calls.remove(friendId);
    emit avCancel(friendId);
}

bool CoreAV::sendCallAudio(uint32_t callId)
{
    if (!calls.contains(callId))
        return false;

    ToxFriendCall& call = calls[callId];

    if (call.muteMic || call.inactive
            || !(call.state & TOXAV_FRIEND_CALL_STATE_ACCEPTING_A)
            || !Audio::getInstance().isInputReady())
    {
        return true;
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

    return true;
}

void CoreAV::sendCallVideo(uint32_t callId, shared_ptr<VideoFrame> vframe)
{
    if (!calls.contains(callId))
        return;

    ToxFriendCall& call = calls[callId];

    if (!call.videoEnabled || call.inactive
            || !(call.state & TOXAV_FRIEND_CALL_STATE_ACCEPTING_V))
        return;

    // This frame shares vframe's buffers, we don't call vpx_img_free but just delete it
    vpx_image* frame = vframe->toVpxImage();
    if (frame->fmt == VPX_IMG_FMT_NONE)
    {
        qWarning() << "Invalid frame";
        delete frame;
        return;
    }

    if (!toxav_video_send_frame(toxav, callId, frame->d_w, frame->d_h,
                                frame->planes[0], frame->planes[1], frame->planes[2], nullptr))
        qDebug() << "toxav_video_send_frame error";

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

    auto call = groupCalls.insert({groupId, *this});
    call->inactive = false;
}

void CoreAV::leaveGroupCall(int groupId)
{
    qDebug() << QString("Leaving group call %1").arg(groupId);

    groupCalls.remove(groupId);
}

bool CoreAV::sendGroupCallAudio(int groupId)
{
    if (!groupCalls.contains(groupId))
        return false;

    ToxGroupCall& call = groupCalls[groupId];

    if (call.inactive || call.muteMic || !Audio::getInstance().isInputReady())
        return true;

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

        if (toxav_group_send_audio(toxav_get_tox(toxav), groupId, buf, AUDIO_FRAME_SAMPLE_COUNT,
                                    AUDIO_CHANNELS, AUDIO_SAMPLE_RATE) != 0)
            qDebug() << "toxav_group_send_audio error";
    }

    return true;
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
        if (call.alSource)
        {
            Audio::deleteSource(&call.alSource);
            Audio::createSource(&call.alSource);
        }
    }

    for (ToxFriendCall& call : calls)
    {
        if (call.alSource)
        {
            Audio::deleteSource(&call.alSource);
            Audio::createSource(&call.alSource);
        }
    }
}

void CoreAV::callCallback(ToxAV* toxav, uint32_t friendNum, bool audio, bool video, void *_self)
{
    CoreAV* self = static_cast<CoreAV*>(_self);
    if (self->calls.contains(friendNum))
    {
        qWarning() << QString("Rejecting call invite from %1, we're already in that call!").arg(friendNum);
        toxav_call_control(toxav, friendNum, TOXAV_CALL_CONTROL_CANCEL, nullptr);
        return;
    }
    const auto& callIt = self->calls.insert({friendNum, video, *self});

    // We don't get a state callback when answering, so fill the state ourselves in advance
    int state = 0;
    if (audio)
        state |= TOXAV_FRIEND_CALL_STATE_SENDING_A | TOXAV_FRIEND_CALL_STATE_ACCEPTING_A;
    if (video)
        state |= TOXAV_FRIEND_CALL_STATE_SENDING_V | TOXAV_FRIEND_CALL_STATE_ACCEPTING_V;
    callIt->state = static_cast<TOXAV_FRIEND_CALL_STATE>(state);

    emit reinterpret_cast<CoreAV*>(self)->avInvite(friendNum, video);
}

void CoreAV::stateCallback(ToxAV *, uint32_t friendNum, uint32_t state, void *_self)
{
    // This callback needs to run in the CoreAV thread.
    // Otherwise, there's a deadlock between the Core thread holding an internal
    // toxav lock and trying to lock the CameraSource to stop it, and the
    // av thread holding the Camera

    CoreAV* self = static_cast<CoreAV*>(_self);

    assert(self->calls.contains(friendNum));
    ToxFriendCall& call = self->calls[friendNum];

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
        // If our state was null, we started the call and were still ringing
        if (!call.state && state)
        {
            call.inactive = false;
            emit self->avStart(friendNum, call.videoEnabled);
        }

        call.state = static_cast<TOXAV_FRIEND_CALL_STATE>(state);
    }
}

void CoreAV::audioBitrateCallback(ToxAV*, uint32_t friendNum, bool stable, uint32_t rate, void *)
{
    qDebug() << "Audio bitrate with"<<friendNum<<" is now "<<rate<<", stability:"<<stable;
}

void CoreAV::videoBitrateCallback(ToxAV*, uint32_t friendNum, bool stable, uint32_t rate, void *)
{
    qDebug() << "Video bitrate with"<<friendNum<<" is now "<<rate<<", stability:"<<stable;
}

void CoreAV::audioFrameCallback(ToxAV *, uint32_t friendNum, const int16_t *pcm,
                                size_t sampleCount, uint8_t channels, uint32_t samplingRate, void *_self)
{
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

void CoreAV::videoFrameCallback(ToxAV *, uint32_t friendNum, uint16_t w, uint16_t h,
                                const uint8_t *y, const uint8_t *u, const uint8_t *v,
                                int32_t ystride, int32_t ustride, int32_t vstride, void *)
{
    if (!calls.contains(friendNum))
        return;

    vpx_image frame;
    frame.d_h = h;
    frame.d_w = w;
    frame.planes[0] = const_cast<uint8_t*>(y);
    frame.planes[1] = const_cast<uint8_t*>(u);
    frame.planes[2] = const_cast<uint8_t*>(v);
    frame.stride[0] = ystride;
    frame.stride[1] = ustride;
    frame.stride[2] = vstride;

    ToxFriendCall& call = calls[friendNum];
    call.videoSource->pushFrame(&frame);
}
