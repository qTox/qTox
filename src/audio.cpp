/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/


#include "audio.h"
#include "src/core.h"

#include <QDebug>
#include <QThread>

#include <cassert>

std::atomic<int> Audio::userCount{0};
Audio* Audio::instance{nullptr};
QThread* Audio::audioThread{nullptr};
ALCdevice* Audio::alInDev{nullptr};
ALCdevice* Audio::alOutDev{nullptr};
ALCcontext* Audio::alContext{nullptr};
ALuint Audio::alMainSource{0};

Audio& Audio::getInstance()
{
    if (!instance)
    {
        instance = new Audio();
        audioThread = new QThread(instance);
        audioThread->setObjectName("qTox Audio");
        audioThread->start();
        instance->moveToThread(audioThread);
    }
    return *instance;
}

void Audio::suscribeInput()
{
    if (!userCount++ && alInDev)
        alcCaptureStart(alInDev);
}

void Audio::unsuscribeInput()
{
    if (!--userCount && alInDev)
        alcCaptureStop(alInDev);
}

void Audio::openInput(const QString& inDevDescr)
{
    auto* tmp = alInDev;
    alInDev = nullptr;
    if (tmp)
        alcCaptureCloseDevice(tmp);
    int stereoFlag = av_DefaultSettings.audio_channels==1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    if (inDevDescr.isEmpty())
        alInDev = alcCaptureOpenDevice(nullptr,av_DefaultSettings.audio_sample_rate, stereoFlag,
            (av_DefaultSettings.audio_frame_duration * av_DefaultSettings.audio_sample_rate * 4)
                                       / 1000 * av_DefaultSettings.audio_channels);
    else
        alInDev = alcCaptureOpenDevice(inDevDescr.toStdString().c_str(),av_DefaultSettings.audio_sample_rate, stereoFlag,
            (av_DefaultSettings.audio_frame_duration * av_DefaultSettings.audio_sample_rate * 4)
                                       / 1000 * av_DefaultSettings.audio_channels);
    if (!alInDev)
        qWarning() << "Audio: Cannot open input audio device";
    else
        qDebug() << "Audio: Opening audio input "<<inDevDescr;

    Core::getInstance()->resetCallSources(); // Force to regen each group call's sources

    // Restart the capture if necessary
    if (userCount.load() != 0 && alInDev)
        alcCaptureStart(alInDev);
}

void Audio::openOutput(const QString& outDevDescr)
{
    auto* tmp = alOutDev;
    alOutDev = nullptr;
    if (tmp)
        alcCloseDevice(tmp);
    if (outDevDescr.isEmpty())
        alOutDev = alcOpenDevice(nullptr);
    else
        alOutDev = alcOpenDevice(outDevDescr.toStdString().c_str());
    if (!alOutDev)
    {
        qWarning() << "Audio: Cannot open output audio device";
    }
    else
    {
        if (alContext)
            alcDestroyContext(alContext);
        alContext=alcCreateContext(alOutDev,nullptr);
        if (!alcMakeContextCurrent(alContext))
        {
            qWarning() << "Audio: Cannot create output audio context";
            alcCloseDevice(alOutDev);
        }
        else
            alGenSources(1, &alMainSource);


        qDebug() << "Audio: Opening audio output "<<outDevDescr;
    }

    Core::getInstance()->resetCallSources(); // Force to regen each group call's sources
}

void Audio::closeInput()
{
    if (alInDev)
        alcCaptureCloseDevice(alInDev);

    userCount = 0;
}

void Audio::closeOutput()
{
    if (alContext)
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(alContext);
    }

    if (alOutDev)
        alcCloseDevice(alOutDev);
}

void Audio::playMono16Sound(const QByteArray& data)
{
    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, data.data(), data.size(), 44100);
    alSourcei(alMainSource, AL_BUFFER, buffer);
    alSourcePlay(alMainSource);
    alDeleteBuffers(1, &buffer);
}

void Audio::playGroupAudioQueued(Tox*,int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate,void*)
{
    QMetaObject::invokeMethod(instance, "playGroupAudio", Qt::BlockingQueuedConnection,
                              Q_ARG(int,group), Q_ARG(int,peer), Q_ARG(const int16_t*,data),
                              Q_ARG(unsigned,samples), Q_ARG(uint8_t,channels), Q_ARG(unsigned,sample_rate));
}

void Audio::playGroupAudio(int group, int peer, const int16_t* data,
                           unsigned samples, uint8_t channels, unsigned sample_rate)
{
    assert(QThread::currentThread() == audioThread);

    ToxGroupCall& call = Core::groupCalls[group];

    if (!call.active || call.muteVol)
        return;

    if (!call.alSources.contains(peer))
        alGenSources(1, &call.alSources[peer]);

    playAudioBuffer(call.alSources[peer], data, samples, channels, sample_rate);
}

void Audio::playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate)
{
    assert(channels == 1 || channels == 2);

    ALuint bufid;
    ALint processed = 0, queued = 16;
    alGetSourcei(alSource, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(alSource, AL_BUFFERS_QUEUED, &queued);
    alSourcei(alSource, AL_LOOPING, AL_FALSE);

    if(processed)
    {
        ALuint bufids[processed];
        alSourceUnqueueBuffers(alSource, processed, bufids);
        alDeleteBuffers(processed - 1, bufids + 1);
        bufid = bufids[0];
    }
    else if(queued < 16)
    {
        alGenBuffers(1, &bufid);
    }
    else
    {
        qDebug() << "Audio: Dropped frame";
        return;
    }

    alBufferData(bufid, (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data,
                    samples * 2 * channels, sampleRate);
    alSourceQueueBuffers(alSource, 1, &bufid);

    ALint state;
    alGetSourcei(alSource, AL_SOURCE_STATE, &state);
    if(state != AL_PLAYING)
        alSourcePlay(alSource);
}
