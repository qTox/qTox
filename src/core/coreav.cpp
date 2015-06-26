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

QVector<ToxCall> CoreAV::calls;
QHash<int, ToxGroupCall> CoreAV::groupCalls;
#ifdef QTOX_FILTER_AUDIO
QVector<AudioFilterer*> CoreAV::filterer;
#endif

CoreAV::~CoreAV()
{
    for (ToxCall call : calls)
    {
        if (!call.active)
            continue;
        hangupCall(call.callId);
    }
}

bool CoreAV::anyActiveCalls()
{
    for (auto& call : calls)
    {
        if (call.active)
            return true;
    }
    return false;
}

void CoreAV::prepareCall(uint32_t friendId, int32_t callId, ToxAV* toxav, bool videoEnabled)
{
    qDebug() << QString("preparing call %1").arg(callId);

    calls[callId].callId = callId;
    calls[callId].friendId = friendId;
    calls[callId].muteMic = false;
    calls[callId].muteVol = false;
    // the following three lines are also now redundant from startCall, but are
    // necessary there for outbound and here for inbound
    calls[callId].videoEnabled = videoEnabled;

    // Audio
    Audio::getInstance().subscribeInput();

    // Go
    calls[callId].active = true;
    calls[callId].sendAudioTimer->setInterval(5);
    calls[callId].sendAudioTimer->setSingleShot(true);
    connect(calls[callId].sendAudioTimer, &QTimer::timeout, [=](){sendCallAudio(callId,toxav);});
    calls[callId].sendAudioTimer->start();

    if (calls[callId].videoEnabled)
    {
        calls[callId].videoSource = new CoreVideoSource;
        CameraSource& source = CameraSource::getInstance();
        source.subscribe();
        connect(&source, &VideoSource::frameAvailable,
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

void CoreAV::answerCall(int32_t callId)
{

}

void CoreAV::hangupCall(int32_t callId)
{
    qDebug() << QString("hanging up call %1").arg(callId);
    calls[callId].active = false;
}

void CoreAV::rejectCall(int32_t callId)
{
    qDebug() << QString("rejecting call %1").arg(callId);
    calls[callId].active = false;
}

void CoreAV::startCall(uint32_t friendId, bool video)
{

}

void CoreAV::cancelCall(int32_t callId, uint32_t friendId)
{
    qDebug() << QString("Cancelling call with %1").arg(friendId);
    calls[callId].active = false;
}

void CoreAV::cleanupCall(int32_t callId)
{
    assert(calls[callId].active);
    qDebug() << QString("cleaning up call %1").arg(callId);
    calls[callId].active = false;
    disconnect(calls[callId].sendAudioTimer,0,0,0);
    calls[callId].sendAudioTimer->stop();

    if (calls[callId].videoEnabled)
    {
        CameraSource::getInstance().unsubscribe();
        if (calls[callId].videoSource)
        {
            calls[callId].videoSource->setDeleteOnClose(true);
            calls[callId].videoSource = nullptr;
        }
    }

    Audio::getInstance().unsubscribeInput();
    //toxav_kill_transmission(Core::getInstance()->toxav, callId);
}

void CoreAV::playCallAudio(void* toxav, int32_t callId, const int16_t *data, uint16_t samples, void *user_data)
{
    Q_UNUSED(user_data);

    if (!calls[callId].active || calls[callId].muteVol)
        return;

    if (!calls[callId].alSource)
        alGenSources(1, &calls[callId].alSource);

    //ToxAvCSettings dest;
    //if (toxav_get_peer_csettings((ToxAV*)toxav, callId, 0, &dest) == 0)
    //    playAudioBuffer(calls[callId].alSource, data, samples, dest.audio_channels, dest.audio_sample_rate);
}

void CoreAV::sendCallAudio(int32_t callId, ToxAV* toxav)
{
    if (!calls[callId].active)
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

    if (!calls[callId].active || !calls[callId].videoEnabled)
        return;

    calls[callId].videoSource->pushFrame(img);
}

void CoreAV::sendCallVideo(int32_t callId, ToxAV* toxav, std::shared_ptr<VideoFrame> vframe)
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

void CoreAV::micMuteToggle(int32_t callId)
{
    if (calls[callId].active)
        calls[callId].muteMic = !calls[callId].muteMic;
}

void CoreAV::volMuteToggle(int32_t callId)
{
    if (calls[callId].active)
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

VideoSource *CoreAV::getVideoSourceFromCall(int callNumber)
{
    return calls[callNumber].videoSource;
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
    Core* core = Core::getInstance();
    ToxAV* toxav = core->toxav;

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

bool CoreAV::isGroupCallMicEnabled(int groupId)
{
    return !groupCalls[groupId].muteMic;
}

bool CoreAV::isGroupCallVolEnabled(int groupId)
{
    return !groupCalls[groupId].muteVol;
}

bool CoreAV::isGroupAvEnabled(int groupId)
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
        if (call.active && call.alSource)
        {
            ALuint tmp = call.alSource;
            call.alSource = 0;
            alDeleteSources(1, &tmp);

            alGenSources(1, &call.alSource);
        }
    }
}
