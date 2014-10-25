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
#include "video/camera.h"
#include <QDebug>
#include <QTimer>

ToxCall Core::calls[TOXAV_MAX_CALLS];
const int Core::videobufsize{TOXAV_MAX_VIDEO_WIDTH * TOXAV_MAX_VIDEO_HEIGHT * 4};
uint8_t* Core::videobuf;
int Core::videoBusyness;

ALCdevice* Core::alOutDev, *Core::alInDev;
ALCcontext* Core::alContext;
ALuint Core::alMainSource;

bool Core::anyActiveCalls()
{
    for (auto& call : calls)
        if (call.active)
            return true;
    return false;
}

void Core::prepareCall(int friendId, int callId, ToxAv* toxav, bool videoEnabled)
{
    qDebug() << QString("Core: preparing call %1").arg(callId);
    calls[callId].callId = callId;
    calls[callId].friendId = friendId;
    calls[callId].muteMic = false;
    // the following three lines are also now redundant from startCall, but are
    // necessary there for outbound and here for inbound
    calls[callId].codecSettings = av_DefaultSettings;
    calls[callId].codecSettings.max_video_width = TOXAV_MAX_VIDEO_WIDTH;
    calls[callId].codecSettings.max_video_height = TOXAV_MAX_VIDEO_HEIGHT;
    calls[callId].videoEnabled = videoEnabled;
    toxav_prepare_transmission(toxav, callId, av_jbufdc, av_VADd, videoEnabled);

    // Audio
    alGenSources(1, &calls[callId].alSource);
    alcCaptureStart(alInDev);

    // Go
    calls[callId].active = true;
    calls[callId].sendAudioTimer->setInterval(5);
    calls[callId].sendAudioTimer->setSingleShot(true);
    connect(calls[callId].sendAudioTimer, &QTimer::timeout, [=](){sendCallAudio(callId,toxav);});
    calls[callId].sendAudioTimer->start();
    calls[callId].sendVideoTimer->setInterval(50);
    calls[callId].sendVideoTimer->setSingleShot(true);
    if (calls[callId].videoEnabled)
    {
        calls[callId].sendVideoTimer->start();
        Camera::getInstance()->subscribe();
    }
}

void Core::onAvMediaChange(void* toxav, int32_t callId, void* core)
{
    ToxAvCSettings settings;
    toxav_get_peer_csettings((ToxAv*)toxav, callId, 0, &settings);
    int friendId = toxav_get_peer_id((ToxAv*)toxav, callId, 0);

    qWarning() << "Core: Received media change from friend "<<friendId;

    if (settings.call_type == TypeAudio)
    {
        calls[callId].videoEnabled = false;
        calls[callId].sendVideoTimer->stop();
        Camera::getInstance()->unsubscribe();
        emit ((Core*)core)->avMediaChange(friendId, callId, false);
    }
    else
    {
        Camera::getInstance()->subscribe();
        calls[callId].videoEnabled = true;
        calls[callId].sendVideoTimer->start();
        emit ((Core*)core)->avMediaChange(friendId, callId, true);
    }
}

void Core::answerCall(int callId)
{
    int friendId = toxav_get_peer_id(toxav, callId, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV answer peer ID";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, callId, 0, transSettings);
    if (err != ErrorNone)
    {
         qWarning() << "Core::answerCall: error getting call settings";
         delete transSettings;
         return;
    }

    if (transSettings->call_type == TypeVideo)
    {
        qDebug() << QString("Core: answering call %1 with video").arg(callId);
        toxav_answer(toxav, callId, transSettings);
    }
    else
    {
        qDebug() << QString("Core: answering call %1 without video").arg(callId);
        toxav_answer(toxav, callId, transSettings);
    }

    delete transSettings;
}

void Core::hangupCall(int callId)
{
    qDebug() << QString("Core: hanging up call %1").arg(callId);
    calls[callId].active = false;
    toxav_hangup(toxav, callId);
}

void Core::startCall(int friendId, bool video)
{
    int callId;
    ToxAvCSettings cSettings = av_DefaultSettings;
    cSettings.max_video_width = TOXAV_MAX_VIDEO_WIDTH;
    cSettings.max_video_height = TOXAV_MAX_VIDEO_HEIGHT;
    if (video)
    {
        qDebug() << QString("Core: Starting new call with %1 with video").arg(friendId);
        cSettings.call_type = TypeVideo;
        if (toxav_call(toxav, &callId, friendId, &cSettings, TOXAV_RINGING_TIME) == 0)
        {
            calls[callId].videoEnabled=true;
        }
        else
        {
            qWarning() << QString("Core: Failed to start new video call with %1").arg(friendId);
            emit avCallFailed(friendId);
            return;
        }
    }
    else
    {
        qDebug() << QString("Core: Starting new call with %1 without video").arg(friendId);
        cSettings.call_type = TypeAudio;
        if (toxav_call(toxav, &callId, friendId, &cSettings, TOXAV_RINGING_TIME) == 0)
        {
            calls[callId].videoEnabled=false;
        }
        else
        {
            qWarning() << QString("Core: Failed to start new audio call with %1").arg(friendId);
            emit avCallFailed(friendId);
            return;
        }

    }
}

void Core::cancelCall(int callId, int friendId)
{
    qDebug() << QString("Core: Cancelling call with %1").arg(friendId);
    calls[callId].active = false;
    toxav_cancel(toxav, callId, friendId, 0);
}

void Core::cleanupCall(int callId)
{
    qDebug() << QString("Core: cleaning up call %1").arg(callId);
    calls[callId].active = false;
    disconnect(calls[callId].sendAudioTimer,0,0,0);
    calls[callId].sendAudioTimer->stop();
    calls[callId].sendVideoTimer->stop();
    if (calls[callId].videoEnabled)
        Camera::getInstance()->unsubscribe();
    alcCaptureStop(alInDev);
}

void Core::playCallAudio(ToxAv* toxav, int32_t callId, int16_t *data, int samples, void *user_data)
{
    Q_UNUSED(user_data);

    if (!calls[callId].active)
        return;

    ToxAvCSettings dest;
    if(toxav_get_peer_csettings(toxav, callId, 0, &dest) == 0)
        playAudioBuffer(callId, data, samples, dest.audio_channels, dest.audio_sample_rate);
}

void Core::sendCallAudio(int callId, ToxAv* toxav)
{
    if (!calls[callId].active)
        return;

    if (calls[callId].muteMic)
    {
        calls[callId].sendAudioTimer->start();
        return;
    }

    int framesize = (calls[callId].codecSettings.audio_frame_duration * calls[callId].codecSettings.audio_sample_rate) / 1000;
    uint8_t buf[framesize*2], dest[framesize*2];

    bool frame = false;
    ALint samples;
    alcGetIntegerv(alInDev, ALC_CAPTURE_SAMPLES, sizeof(samples), &samples);
    if(samples >= framesize)
    {
        alcCaptureSamples(alInDev, buf, framesize);
        frame = 1;
    }

    if(frame)
    {
        int r;
        if((r = toxav_prepare_audio_frame(toxav, callId, dest, framesize*2, (int16_t*)buf, framesize)) < 0)
        {
            qDebug() << "Core: toxav_prepare_audio_frame error";
            calls[callId].sendAudioTimer->start();
            return;
        }

        if((r = toxav_send_audio(toxav, callId, dest, r)) < 0)
            qDebug() << "Core: toxav_send_audio error";
    }
    calls[callId].sendAudioTimer->start();
}

void Core::playCallVideo(ToxAv*, int32_t callId, vpx_image_t* img, void *user_data)
{
    Q_UNUSED(user_data);

    if (!calls[callId].active || !calls[callId].videoEnabled)
        return;

    calls[callId].videoSource.pushVPXFrame(img);

    vpx_img_free(img);
}

void Core::sendCallVideo(int callId)
{
    if (!calls[callId].active || !calls[callId].videoEnabled)
        return;

    vpx_image frame = camera->getLastFrame().createVpxImage();
    if (frame.w && frame.h)
    {
        int result;
        if((result = toxav_prepare_video_frame(toxav, callId, videobuf, videobufsize, &frame)) < 0)
        {
            qDebug() << QString("Core: toxav_prepare_video_frame: error %1").arg(result);
            vpx_img_free(&frame);
            calls[callId].sendVideoTimer->start();
            return;
        }

        if((result = toxav_send_video(toxav, callId, (uint8_t*)videobuf, result)) < 0)
            qDebug() << QString("Core: toxav_send_video error: %1").arg(result);

        vpx_img_free(&frame);
    }
    else
    {
        qDebug("Core::sendCallVideo: Invalid frame (bad camera ?)");
    }

    calls[callId].sendVideoTimer->start();
}


void Core::increaseVideoBusyness()
{
    videoBusyness++;
}

void Core::decreaseVideoBusyness()
{
  videoBusyness--;
}

void Core::micMuteToggle(int callId)
{
    if (calls[callId].active) {
        calls[callId].muteMic = !calls[callId].muteMic;
    }
}

void Core::onAvCancel(void* _toxav, int32_t callId, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, callId, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV cancel";
        return;
    }
    qDebug() << QString("Core: AV cancel from %1").arg(friendId);

    calls[callId].active = false;

    emit static_cast<Core*>(core)->avCancel(friendId, callId);
}

void Core::onAvReject(void*, int32_t, void*)
{
    qDebug() << "Core: AV reject";
}

void Core::onAvEnd(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV end";
        return;
    }
    qDebug() << QString("Core: AV end from %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avEnd(friendId, call_index);
}

void Core::onAvRinging(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV ringing";
        return;
    }

    if (calls[call_index].videoEnabled)
    {
        qDebug() << QString("Core: AV ringing with %1 with video").arg(friendId);
        emit static_cast<Core*>(core)->avRinging(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV ringing with %1 without video").arg(friendId);
        emit static_cast<Core*>(core)->avRinging(friendId, call_index, false);
    }
}

void Core::onAvStarting(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV starting";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, call_index, 0, transSettings);
    if (err != ErrorNone)
    {
        qWarning() << "Core::onAvStarting: error getting call type";
        delete transSettings;
        return;
    }

    if (transSettings->call_type == TypeVideo)
    {
        qDebug() << QString("Core: AV starting from %1 with video").arg(friendId);
        prepareCall(friendId, call_index, toxav, true);
        emit static_cast<Core*>(core)->avStarting(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV starting from %1 without video").arg(friendId);
        prepareCall(friendId, call_index, toxav, false);
        emit static_cast<Core*>(core)->avStarting(friendId, call_index, false);
    }

    delete transSettings;
}

void Core::onAvEnding(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV ending";
        return;
    }
    qDebug() << QString("Core: AV ending from %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avEnding(friendId, call_index);
}

void Core::onAvRequestTimeout(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV request timeout";
        return;
    }
    qDebug() << QString("Core: AV request timeout with %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avRequestTimeout(friendId, call_index);
}

void Core::onAvPeerTimeout(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV peer timeout";
        return;
    }
    qDebug() << QString("Core: AV peer timeout with %1").arg(friendId);

    cleanupCall(call_index);

    emit static_cast<Core*>(core)->avPeerTimeout(friendId, call_index);
}


void Core::onAvInvite(void* _toxav, int32_t call_index, void* core)
{
    ToxAv* toxav = static_cast<ToxAv*>(_toxav);

    int friendId = toxav_get_peer_id(toxav, call_index, 0);
    if (friendId < 0)
    {
        qWarning() << "Core: Received invalid AV invite";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, call_index, 0, transSettings);
    if (err != ErrorNone)
    {
        qWarning() << "Core::onAvInvite: error getting call type";
        delete transSettings;
        return;
    }

    if (transSettings->call_type == TypeVideo)
    {
        qDebug() << QString("Core: AV invite from %1 with video").arg(friendId);
        emit static_cast<Core*>(core)->avInvite(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV invite from %1 without video").arg(friendId);
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
        qWarning() << "Core: Received invalid AV start";
        return;
    }

    ToxAvCSettings* transSettings = new ToxAvCSettings;
    int err = toxav_get_peer_csettings(toxav, call_index, 0, transSettings);
    if (err != ErrorNone)
    {
        qWarning() << "Core::onAvStart: error getting call type";
        delete transSettings;
        return;
    }

    if (transSettings->call_type == TypeVideo)
    {
        qDebug() << QString("Core: AV start from %1 with video").arg(friendId);
        prepareCall(friendId, call_index, toxav, true);
        emit static_cast<Core*>(core)->avStart(friendId, call_index, true);
    }
    else
    {
        qDebug() << QString("Core: AV start from %1 without video").arg(friendId);
        prepareCall(friendId, call_index, toxav, false);
        emit static_cast<Core*>(core)->avStart(friendId, call_index, false);
    }

    delete transSettings;
}

// This function's logic was shamelessly stolen from uTox
void Core::playAudioBuffer(int callId, int16_t *data, int samples, unsigned channels, int sampleRate)
{
    if(!channels || channels > 2)
    {
        qWarning() << "Core::playAudioBuffer: trying to play on "<<channels<<" channels! Giving up.";
        return;
    }

    ALuint bufid;
    ALint processed, queued;
    alGetSourcei(calls[callId].alSource, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(calls[callId].alSource, AL_BUFFERS_QUEUED, &queued);
    alSourcei(calls[callId].alSource, AL_LOOPING, AL_FALSE);

    if(processed)
    {
        ALuint bufids[processed];
        alSourceUnqueueBuffers(calls[callId].alSource, processed, bufids);
        alDeleteBuffers(processed - 1, bufids + 1);
        bufid = bufids[0];
    }
    else if(queued < 16)
    {
        alGenBuffers(1, &bufid);
    }
    else
    {
        qDebug() << "Core: Dropped audio frame";
        return;
    }

    alBufferData(bufid, (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data,
                    samples * 2 * channels, sampleRate);
    alSourceQueueBuffers(calls[callId].alSource, 1, &bufid);

    ALint state;
    alGetSourcei(calls[callId].alSource, AL_SOURCE_STATE, &state);
    if(state != AL_PLAYING)
    {
        alSourcePlay(calls[callId].alSource);
        qDebug() << "Core: Starting audio source of call " << callId;
    }
}

VideoSource *Core::getVideoSourceFromCall(int callNumber)
{
    return &calls[callNumber].videoSource;
}

