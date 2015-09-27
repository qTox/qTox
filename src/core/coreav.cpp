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

QHash<uint32_t, ToxCall> CoreAV::calls;
QHash<int, ToxGroupCall> CoreAV::groupCalls;

ToxCall::ToxCall(uint32_t FriendNum, bool VideoEnabled, CoreAV& av)
    : sendAudioTimer{new QTimer}, friendNum{FriendNum},
      muteMic{false}, muteVol{false},
      videoEnabled{VideoEnabled},
      alSource{0}, videoSource{nullptr},
      state{static_cast<TOXAV_FRIEND_CALL_STATE>(0)}
{
    Audio::getInstance().subscribeInput();
    sendAudioTimer->setInterval(5);
    sendAudioTimer->setSingleShot(true);
    QObject::connect(sendAudioTimer, &QTimer::timeout, [=,&av](){av.sendCallAudio(friendNum);});
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

ToxCall::~ToxCall()
{
    QObject::disconnect(sendAudioTimer, nullptr, nullptr, nullptr);
    sendAudioTimer->stop();
    if (videoEnabled)
    {
        CameraSource::getInstance().unsubscribe();
        if (videoSource)
        {
            videoSource->setDeleteOnClose(true);
            videoSource = nullptr;
        }
    }

    Audio::getInstance().unsubscribeInput();
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
    for (ToxCall call : calls)
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

    calls.insert(friendId, {friendId, video, *this});
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

    if (calls[callId].muteMic || !Audio::getInstance().isInputReady())
    {
        calls[callId].sendAudioTimer->start();
        return;
    }

#if 0
    const int framesize = (calls[callId].codecSettings.audio_frame_duration * calls[callId].codecSettings.audio_sample_rate) / 1000 * av_DefaultSettings.audio_channels;
    const int bufsize = framesize * 2 * av_DefaultSettings.audio_channels;
    uint8_t buf[bufsize];

    if (Audio::getInstance().tryCaptureSamples(buf, framesize))
    {
#ifdef QTOX_FILTER_AUDIO
        if (Settings::getInstance().getFilterAudio())
        {
            if (!filterer[callId])
            {
                filterer[callId] = new AudioFilterer();
                filterer[callId]->startFilter(48000);
            }
            // is a null op #ifndef ALC_LOOPBACK_CAPTURE_SAMPLES
            Audio::getEchoesToFilter(filterer[callId], framesize);

            filterer[callId]->filterAudio((int16_t*) buf, framesize);
        }
        else if (filterer[callId])
        {
            delete filterer[callId];
            filterer[callId] = nullptr;
        }
#endif

        uint8_t dest[bufsize];
        int r;
        if ((r = toxav_prepare_audio_frame(toxav, callId, dest, framesize*2, (int16_t*)buf, framesize)) < 0)
        {
            qDebug() << "toxav_prepare_audio_frame error";
            calls[callId].sendAudioTimer->start();
            return;
        }

        if ((r = toxav_send_audio(toxav, callId, dest, r)) < 0)
            qDebug() << "toxav_send_audio error";
    }
#endif
    calls[callId].sendAudioTimer->start();
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

// This function's logic was shamelessly stolen from uTox
void CoreAV::playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate)
{
    if (!channels || channels > 2)
    {
        qWarning() << "playAudioBuffer: trying to play on "<<channels<<" channels! Giving up.";
        return;
    }

    ALuint bufid;
    ALint processed = 0, queued = 16;
    alGetSourcei(alSource, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(alSource, AL_BUFFERS_QUEUED, &queued);
    alSourcei(alSource, AL_LOOPING, AL_FALSE);

    if (processed)
    {
        ALuint bufids[processed];
        alSourceUnqueueBuffers(alSource, processed, bufids);
        alDeleteBuffers(processed - 1, bufids + 1);
        bufid = bufids[0];
    }
    else if (queued < 32)
    {
        alGenBuffers(1, &bufid);
    }
    else
    {
        qDebug() << "Dropped audio frame";
        return;
    }

    alBufferData(bufid, (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data,
                    samples * 2 * channels, sampleRate);
    alSourceQueueBuffers(alSource, 1, &bufid);

    ALint state;
    alGetSourcei(alSource, AL_SOURCE_STATE, &state);
    alSourcef(alSource, AL_GAIN, Audio::getInstance().getOutputVolume());
    if (state != AL_PLAYING)
    {
        alSourcePlay(alSource);
        //qDebug() << "Starting audio source " << (int)alSource;
    }
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

void CoreAV::callCallback(ToxAV*, uint32_t friendNum, bool, bool video, void *_self)
{
    qWarning() << "RECEIVED CALL";
    CoreAV* self = static_cast<CoreAV*>(_self);
    calls.insert(friendNum, {friendNum, video, *self});
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
        // If our state was null, we were ringing and the call just started
        if (!call.state && state)
            emit self->avStart(friendNum, call.videoEnabled);

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

void CoreAV::audioFrameCallback(ToxAV *toxAV, uint32_t friendNum, const int16_t *pcm, size_t sampleCount, uint8_t channels, uint32_t samplingRate, void *self)
{
    qWarning() << "AUDIO FRAME";
}

void CoreAV::videoFrameCallback(ToxAV *toxAV, uint32_t friendNum, uint16_t w, uint16_t h, const uint8_t *y, const uint8_t *u, const uint8_t *v, int32_t ystride, int32_t ustride, int32_t vstride, void *self)
{
    qWarning() << "VIDEO FRAME";
}
