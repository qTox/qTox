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

std::atomic<int> Audio::userCount{0};
ALCdevice* Audio::alInDev{nullptr};
ALCdevice* Audio::alOutDev{nullptr};
ALCcontext* Audio::alContext{0};
ALuint Audio::alMainSource{0};

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
    if (userCount.load() != 0)
        alcCaptureStart(alInDev);
}

void Audio::openOutput(const QString& outDevDescr)
{
    auto* tmp = alOutDev;
    alOutDev = nullptr;
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
