/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>

    This file is part of Tox Qt GUI.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include "core.h"
#include "src/video/camerasource.h"
#include "src/video/corevideosource.h"
#include "src/video/videoframe.h"
#include "src/audio.h"
#ifdef QTOX_FILTER_AUDIO
#include "src/audiofilterer.h"
#endif
#include "src/misc/settings.h"
#include <QDebug>
#include <QTimer>

ToxCall Core::calls[TOXAV_MAX_CALLS];
#ifdef QTOX_FILTER_AUDIO
AudioFilterer * Core::filterer[TOXAV_MAX_CALLS] {nullptr};
#endif
const int Core::videobufsize{TOXAV_MAX_VIDEO_WIDTH * TOXAV_MAX_VIDEO_HEIGHT * 4};
uint8_t* Core::videobuf;

bool Core::anyActiveCalls()
{
    for (auto& call : calls)
    {
        if (call.active)
            return true;
    }
    return false;
}

void Core::prepareCall(uint32_t friendId, int32_t callId, ToxAv* toxav, bool videoEnabled)
{
    qDebug() << QString("preparing call %1").arg(callId);

    if (!videobuf)
        videobuf = new uint8_t[videobufsize];

    calls[callId].callId = callId;
    calls[callId].friendId = friendId;
    calls[callId].muteMic = false;
    calls[callId].muteVol = false;
    // the following three lines are also now redundant from startCall, but are
    // necessary there for outbound and here for inbound
    calls[callId].codecSettings = av_DefaultSettings;
    calls[callId].codecSettings.max_video_width = TOXAV_MAX_VIDEO_WIDTH;
    calls[callId].codecSettings.max_video_height = TOXAV_MAX_VIDEO_HEIGHT;
    calls[callId].videoEnabled = videoEnabled;
    int r = toxav_prepare_transmission(toxav, callId, videoEnabled);
    if (r < 0)
        qWarning() << QString("Error starting call %1: toxav_prepare_transmission failed with %2").arg(callId).arg(r);

    // Audio
    Audio::suscribeInput();

    // Go
    calls[callId].active = true;
    calls[callId].sendAudioTimer->setInterval(5);
    calls[callId].sendAudioTimer->setSingleShot(true);
    connect(calls[callId].sendAudioTimer, &QTimer::timeout, [=](){sendCallAudio(callId,toxav);});
    calls[callId].sendAudioTimer->start();
    if (calls[callId].videoEnabled)
    {
        calls[callId].videoSource = new CoreVideoSource;
        calls[callId].camera = new CameraSource;
        calls[callId].camera->subscribe();
        connect(calls[callId].camera, &VideoSource::frameAvailable,
                [=](std::shared_ptr<VideoFrame> frame){sendCallVideo(callId,toxav,frame);});
    }

#ifdef QTOX_FILTER_AUDIO
    if (Settings::getInstance().getFilterAudio())
    {
        filterer[callId] = new AudioFilterer();
        filterer[callId]->startFilter(48000);
    }
    else
    {
        delete filterer[callId];
        filterer[callId] = nullptr;
    }
#endif
}

void Core::onAvMediaChange(void* toxav, int32_t callId, void* core)
{
    int friendId;
    int cap = toxav_capability_supported((ToxAv*)toxav, callId,
                        (ToxAvCapabilities)(av_VideoEncoding|av_VideoDecoding));
    if (!cap)
        goto fail;

    friendId  = toxav_get_peer_id((ToxAv*)toxav, callId, 0);
    if (friendId < 0)
        goto fail;

    qDebug() << "Received media change from friend "<<friendId;

    if (cap == (av_VideoEncoding|av_VideoDecoding)) // Video call
    {
        emit static_cast<Core*>(core)->avMediaChange(friendId, callId, true);
        calls[callId].videoSource = new CoreVideoSource;
        calls[callId].camera = new CameraSource;
        calls[callId].camera->subscribe();
        calls[callId].videoEnabled = true;
    }
    else // Audio call
    {
        emit static_cast<Core*>(core)->avMediaChange(friendId, callId, false);
        calls[callId].videoEnabled = false;
        delete calls[callId].camera;
        calls[callId].camera = nullptr;
        calls[callId].videoSource->setDeleteOnClose(true);
        calls[callId].videoSource = nullptr;
    }

    return;

fail: // Centralized error handling
    qWarning() << "Toxcore error while receiving media change on call "<<callId;
    return;
}

void Core::answerCall(int32_t callId)
{
    int friendId = toxav_get_peer_id(toxav, callId, 0);
    if (friendId < 0)
    {
        qWarning() << "Received invalid AV answer peer ID";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, callId, 0, transSettings);
    if (err != av_ErrorNone)
    {
         qWarning() << "answerCall: error getting call settings";
         delete transSettings;
         return;
    }

    if (transSettings->call_type == av_TypeVideo)
    {
        qDebug() << QString("answering call %1 with video").arg(callId);
        toxav_answer(toxav, callId, transSettings);
    }
    else
    {
        qDebug() << QString("answering call %1 without video").arg(callId);
        toxav_answer(toxav, callId, transSettings);
    }

    delete transSettings;
}

void Core::hangupCall(int32_t callId)
{
    qDebug() << QString("hanging up call %1").arg(callId);
    calls[callId].active = false;
    toxav_hangup(toxav, callId);
}

void Core::rejectCall(int32_t callId)
{
    qDebug() << QString("rejecting call %1").arg(callId);
    calls[callId].active = false;
    toxav_reject(toxav, callId, nullptr);
}

void Core::startCall(uint32_t friendId, bool video)
{
    int32_t callId;
    ToxAvCSettings cSettings = av_DefaultSettings;
    cSettings.max_video_width = TOXAV_MAX_VIDEO_WIDTH;
    cSettings.max_video_height = TOXAV_MAX_VIDEO_HEIGHT;
    if (video)
    {
        qDebug() << QString("Starting new call with %1 with video").arg(friendId);
        cSettings.call_type = av_TypeVideo;
        if (toxav_call(toxav, &callId, friendId, &cSettings, TOXAV_RINGING_TIME) == 0)
        {
            calls[callId].videoEnabled=true;
        }
        else
        {
            qWarning() << QString("Failed to start new video call with %1").arg(friendId);
            emit avCallFailed(friendId);
            return;
        }
    }
    else
    {
        qDebug() << QString("Starting new call with %1 without video").arg(friendId);
        cSettings.call_type = av_TypeAudio;
        if (toxav_call(toxav, &callId, friendId, &cSettings, TOXAV_RINGING_TIME) == 0)
        {
            calls[callId].videoEnabled=false;
        }
        else
        {
            qWarning() << QString("Failed to start new audio call with %1").arg(friendId);
            emit avCallFailed(friendId);
            return;
        }
    }
}

void Core::cancelCall(int32_t callId, uint32_t friendId)
{
    qDebug() << QString("Cancelling call with %1").arg(friendId);
    calls[callId].active = false;
    toxav_cancel(toxav, callId, friendId, nullptr);
}

void Core::cleanupCall(int32_t callId)
{
    qDebug() << QString("cleaning up call %1").arg(callId);
    calls[callId].active = false;
    disconnect(calls[callId].sendAudioTimer,0,0,0);
    calls[callId].sendAudioTimer->stop();
    if (calls[callId].videoEnabled)
    {
        delete calls[callId].camera;
        calls[callId].camera = nullptr;
        calls[callId].videoSource->setDeleteOnClose(true);
        calls[callId].videoSource = nullptr;
    }

    Audio::unsuscribeInput();
    toxav_kill_transmission(Core::getInstance()->toxav, callId);

    if (!anyActiveCalls())
    {
        delete[] videobuf;
        videobuf = nullptr;
    }
}

void Core::playCallAudio(void* toxav, int32_t callId, const int16_t *data, uint16_t samples, void *user_data)
{
    Q_UNUSED(user_data);

    if (!calls[callId].active || calls[callId].muteVol)
        return;

    if (!calls[callId].alSource)
        alGenSources(1, &calls[callId].alSource);

    ToxAvCSettings dest;
    if (toxav_get_peer_csettings((ToxAv*)toxav, callId, 0, &dest) == 0)
        playAudioBuffer(calls[callId].alSource, data, samples, dest.audio_channels, dest.audio_sample_rate);
}

void Core::sendCallAudio(int32_t callId, ToxAv* toxav)
{
    if (!calls[callId].active)
        return;

    if (calls[callId].muteMic || !Audio::isInputReady())
    {
        calls[callId].sendAudioTimer->start();
        return;
    }

    const int framesize = (calls[callId].codecSettings.audio_frame_duration * calls[callId].codecSettings.audio_sample_rate) / 1000 * av_DefaultSettings.audio_channels;
    const int bufsize = framesize * 2 * av_DefaultSettings.audio_channels;
    uint8_t buf[bufsize];

    if (Audio::tryCaptureSamples(buf, framesize))
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
    calls[callId].sendAudioTimer->start();
}

void Core::playCallVideo(void*, int32_t callId, const vpx_image_t* img, void *user_data)
{
    Q_UNUSED(user_data);

    if (!calls[callId].active || !calls[callId].videoEnabled)
        return;

    calls[callId].videoSource->pushFrame(img);
}

void Core::sendCallVideo(int32_t callId, ToxAv* toxav, std::shared_ptr<VideoFrame> vframe)
{
    if (!calls[callId].active || !calls[callId].videoEnabled)
        return;

    // This frame shares vframe's buffers, we don't call vpx_img_free but just delete it
    vpx_image* frame = vframe->toVpxImage();
    if (frame->fmt == VPX_IMG_FMT_NONE)
    {
        qWarning() << "Invalid frame";
        delete frame;
        return;
    }

    int result;
    if ((result = toxav_prepare_video_frame(toxav, callId, videobuf, videobufsize, frame)) < 0)
    {
        qDebug() << QString("toxav_prepare_video_frame: error %1").arg(result);
        delete frame;
        return;
    }

    if ((result = toxav_send_video(toxav, callId, (uint8_t*)videobuf, result)) < 0)
        qDebug() << QString("toxav_send_video error: %1").arg(result);

    delete frame;
}

void Core::micMuteToggle(int32_t callId)
{
    if (calls[callId].active)
        calls[callId].muteMic = !calls[callId].muteMic;
}

void Core::volMuteToggle(int32_t callId)
{
    if (calls[callId].active)
        calls[callId].muteVol = !calls[callId].muteVol;
}

void Core::onAvCancel(void* _toxav, int32_t callId, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, callId, 0);
    if (friendId < 0)
    {
        qWarning() << "Received invalid AV cancel";
        return;
    }
    qDebug() << QString("AV cancel from %1").arg(friendId);

    calls[callId].active = false;

#ifdef QTOX_FILTER_AUDIO
    if (filterer[callId])
    {
        filterer[callId]->closeFilter();
        delete filterer[callId];
        filterer[callId] = nullptr;
    }
#endif

    emit static_cast<Core*>(core)->avCancel(friendId, callId);
}

void Core::onAvReject(void* _toxav, int32_t callId, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);
    int friendId = toxav_get_peer_id(toxav, callId, 0);
    if (friendId < 0)
    {
        qWarning() << "Received invalid AV reject";
        return;
    }

    qDebug() << QString("AV reject from %1").arg(friendId);

    emit static_cast<Core*>(core)->avRejected(friendId, callId);
}

void Core::onAvEnd(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Received invalid AV end";
        return;
    }
    qDebug() << QString("AV end from %1").arg(friendId);

    emit static_cast<Core*>(core)->avEnd(friendId, call_index);

    cleanupCall(call_index);
}

void Core::onAvRinging(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Received invalid AV ringing";
        return;
    }

    if (calls[call_index].videoEnabled)
    {
        qDebug() << QString("AV ringing with %1 with video").arg(friendId);
        emit static_cast<Core*>(core)->avRinging(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("AV ringing with %1 without video").arg(friendId);
        emit static_cast<Core*>(core)->avRinging(friendId, call_index, false);
    }
}

void Core::onAvRequestTimeout(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Received invalid AV request timeout";
        return;
    }
    qDebug() << QString("AV request timeout with %1").arg(friendId);

    emit static_cast<Core*>(core)->avRequestTimeout(friendId, call_index);

    cleanupCall(call_index);
}

void Core::onAvPeerTimeout(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Received invalid AV peer timeout";
        return;
    }
    qDebug() << QString("AV peer timeout with %1").arg(friendId);

    emit static_cast<Core*>(core)->avPeerTimeout(friendId, call_index);

    cleanupCall(call_index);
}


void Core::onAvInvite(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Received invalid AV invite";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, call_index, 0, transSettings);
    if (err != av_ErrorNone)
    {
        qWarning() << "onAvInvite: error getting call type";
        delete transSettings;
        return;
    }

    if (transSettings->call_type == av_TypeVideo)
    {
        qDebug() << QString("AV invite from %1 with video").arg(friendId);
        emit static_cast<Core*>(core)->avInvite(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("AV invite from %1 without video").arg(friendId);
        emit static_cast<Core*>(core)->avInvite(friendId, call_index, false);
    }

    delete transSettings;
}

void Core::onAvStart(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Received invalid AV start";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, call_index, 0, transSettings);
    if (err != av_ErrorNone)
    {
        qWarning() << "onAvStart: error getting call type";
        delete transSettings;
        return;
    }

    if (transSettings->call_type == av_TypeVideo)
    {
        qDebug() << QString("AV start from %1 with video").arg(friendId);
        prepareCall(friendId, call_index, toxav, true);
        emit static_cast<Core*>(core)->avStart(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("AV start from %1 without video").arg(friendId);
        prepareCall(friendId, call_index, toxav, false);
        emit static_cast<Core*>(core)->avStart(friendId, call_index, false);
    }

    delete transSettings;
}

// This function's logic was shamelessly stolen from uTox
void Core::playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate)
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
    alSourcef(alSource, AL_GAIN, Audio::getOutputVolume());
    if (state != AL_PLAYING)
    {
        alSourcePlay(alSource);
        //qDebug() << "Starting audio source " << (int)alSource;
    }
}

VideoSource *Core::getVideoSourceFromCall(int callNumber)
{
    return calls[callNumber].videoSource;
}

void Core::joinGroupCall(int groupId)
{
    qDebug() << QString("Joining group call %1").arg(groupId);
    groupCalls[groupId].groupId = groupId;
    groupCalls[groupId].muteMic = false;
    groupCalls[groupId].muteVol = false;
    // the following three lines are also now redundant from startCall, but are
    // necessary there for outbound and here for inbound
    groupCalls[groupId].codecSettings = av_DefaultSettings;
    groupCalls[groupId].codecSettings.max_video_width = TOXAV_MAX_VIDEO_WIDTH;
    groupCalls[groupId].codecSettings.max_video_height = TOXAV_MAX_VIDEO_HEIGHT;

    // Audio
    Audio::suscribeInput();

    // Go
    Core* core = Core::getInstance();
    ToxAv* toxav = core->toxav;

    groupCalls[groupId].sendAudioTimer = new QTimer();
    groupCalls[groupId].active = true;
    groupCalls[groupId].sendAudioTimer->setInterval(5);
    groupCalls[groupId].sendAudioTimer->setSingleShot(true);
    connect(groupCalls[groupId].sendAudioTimer, &QTimer::timeout, [=](){sendGroupCallAudio(groupId,toxav);});
    groupCalls[groupId].sendAudioTimer->start();
}

void Core::leaveGroupCall(int groupId)
{
    qDebug() << QString("Leaving group call %1").arg(groupId);
    groupCalls[groupId].active = false;
    disconnect(groupCalls[groupId].sendAudioTimer,0,0,0);
    groupCalls[groupId].sendAudioTimer->stop();
    for (ALuint source : groupCalls[groupId].alSources)
        alDeleteSources(1, &source);
    groupCalls[groupId].alSources.clear();
    Audio::unsuscribeInput();
    delete groupCalls[groupId].sendAudioTimer;
}

void Core::sendGroupCallAudio(int groupId, ToxAv* toxav)
{
    if (!groupCalls[groupId].active)
        return;

    if (groupCalls[groupId].muteMic || !Audio::isInputReady())
    {
        groupCalls[groupId].sendAudioTimer->start();
        return;
    }

    const int framesize = (groupCalls[groupId].codecSettings.audio_frame_duration * groupCalls[groupId].codecSettings.audio_sample_rate) / 1000 * av_DefaultSettings.audio_channels;
    const int bufsize = framesize * 2 * av_DefaultSettings.audio_channels;
    uint8_t buf[bufsize];

    if (Audio::tryCaptureSamples(buf, framesize))
    {
        if (toxav_group_send_audio(toxav_get_tox(toxav), groupId, (int16_t*)buf,
                framesize, av_DefaultSettings.audio_channels, av_DefaultSettings.audio_sample_rate) < 0)
        {
            qDebug() << "toxav_group_send_audio error";
            groupCalls[groupId].sendAudioTimer->start();
            return;
        }
    }
    groupCalls[groupId].sendAudioTimer->start();
}

void Core::disableGroupCallMic(int groupId)
{
    groupCalls[groupId].muteMic = true;
}

void Core::disableGroupCallVol(int groupId)
{
    groupCalls[groupId].muteVol = true;
}

void Core::enableGroupCallMic(int groupId)
{
    groupCalls[groupId].muteMic = false;
}

void Core::enableGroupCallVol(int groupId)
{
    groupCalls[groupId].muteVol = false;
}

bool Core::isGroupCallMicEnabled(int groupId)
{
    return !groupCalls[groupId].muteMic;
}

bool Core::isGroupCallVolEnabled(int groupId)
{
    return !groupCalls[groupId].muteVol;
}
