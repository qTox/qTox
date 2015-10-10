/*
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


// Output some extra debug info
#define AUDIO_DEBUG 1

// Fix a 7 years old openal-soft/alsa bug
// http://blog.gmane.org/gmane.comp.lib.openal.devel/month=20080501
// If set to 1, the capture will be started as long as the device is open
#define FIX_SND_PCM_PREPARE_BUG 0

#include "audio.h"
#include "src/core/core.h"

#include <QDebug>
#include <QThread>
#include <QMutexLocker>

#include <cassert>

std::atomic<int> Audio::userCount{0};
Audio* Audio::instance{nullptr};
QThread* Audio::audioThread{nullptr};
QMutex* Audio::audioInLock{nullptr};
QMutex* Audio::audioOutLock{nullptr};
ALCdevice* Audio::alInDev{nullptr};
ALCdevice* Audio::alOutDev{nullptr};
ALCcontext* Audio::alContext{nullptr};
ALuint Audio::alMainSource{0};
float Audio::outputVolume{1.0};

Audio& Audio::getInstance()
{
    if (!instance)
    {
        instance = new Audio();
        audioThread = new QThread(instance);
        audioThread->setObjectName("qTox Audio");
        audioThread->start();
        audioInLock = new QMutex(QMutex::Recursive);
        audioOutLock = new QMutex(QMutex::Recursive);
        instance->moveToThread(audioThread);
    }
    return *instance;
}

Audio::~Audio()
{
    audioThread->exit(0);
    audioThread->wait();
    if (audioThread->isRunning())
        audioThread->terminate();

    delete audioThread;
    delete audioInLock;
    delete audioOutLock;
}

float Audio::getOutputVolume()
{
    return outputVolume;
}

void Audio::setOutputVolume(float volume)
{
    outputVolume = volume;
    alSourcef(alMainSource, AL_GAIN, outputVolume);

    for (const ToxGroupCall& call : Core::groupCalls)
    {
        if (!call.active)
            continue;
        for (ALuint source : call.alSources)
            alSourcef(source, AL_GAIN, outputVolume);
    }

    for (const ToxCall& call : Core::calls)
    {
        if (!call.active)
            continue;
        alSourcef(call.alSource, AL_GAIN, outputVolume);
    }
}

void Audio::suscribeInput()
{
    if (!alInDev)
    {
        qWarning()<<"input device is closed";
        return;
    }

    qDebug() << "suscribing input";
    QMutexLocker lock(audioInLock);
    if (!userCount++ && alInDev)
    {
#if (!FIX_SND_PCM_PREPARE_BUG)
        qDebug() << "starting capture";
        alcCaptureStart(alInDev);
#endif
    }
}

void Audio::unsuscribeInput()
{
    if (!alInDev)
    {
        qWarning()<<"input device is closed";
        return;
    }

    qDebug() << "unsuscribing input";
    QMutexLocker lock(audioInLock);
    if (!--userCount && alInDev)
    {
#if (!FIX_SND_PCM_PREPARE_BUG)
        qDebug() << "stopping capture";
        alcCaptureStop(alInDev);
#endif
    }
}

void Audio::openInput(const QString& inDevDescr)
{
    QMutexLocker lock(audioInLock);
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
        qWarning() << "Cannot open input audio device " + inDevDescr;
    else
        qDebug() << "Opening audio input "<<inDevDescr;

    Core* core = Core::getInstance();
    if (core)
        core->resetCallSources(); // Force to regen each group call's sources

    // Restart the capture if necessary
    if (userCount.load() != 0 && alInDev)
    {
        alcCaptureStart(alInDev);
    }
    else
    {
#if (FIX_SND_PCM_PREPARE_BUG)
    alcCaptureStart(alInDev);
#endif
    }

}

void Audio::openOutput(const QString& outDevDescr)
{
    QMutexLocker lock(audioOutLock);
    auto* tmp = alOutDev;
    alOutDev = nullptr;
    if (outDevDescr.isEmpty())
        alOutDev = alcOpenDevice(nullptr);
    else
        alOutDev = alcOpenDevice(outDevDescr.toStdString().c_str());

    if (!alOutDev)
    {
        qWarning() << "Cannot open output audio device " + outDevDescr;
    }
    else
    {
        if (alContext && alcMakeContextCurrent(nullptr) == ALC_TRUE)
            alcDestroyContext(alContext);

        if (tmp)
            alcCloseDevice(tmp);

        alContext=alcCreateContext(alOutDev,nullptr);
        if (!alcMakeContextCurrent(alContext))
        {
            qWarning() << "Cannot create output audio context";
            alcCloseDevice(alOutDev);
        }
        else
        {
            alGenSources(1, &alMainSource);
        }


        qDebug() << "Opening audio output " + outDevDescr;
    }

    Core* core = Core::getInstance();
    if (core)
        core->resetCallSources(); // Force to regen each group call's sources
}

void Audio::closeInput()
{
    qDebug() << "Closing input";
    QMutexLocker lock(audioInLock);
    if (alInDev)
    {
        if (alcCaptureCloseDevice(alInDev) == ALC_TRUE)
        {
            alInDev = nullptr;
            userCount = 0;
        }
        else
        {
            qWarning() << "Failed to close input";
        }
    }
}

void Audio::closeOutput()
{
    qDebug() << "Closing output";
    QMutexLocker lock(audioOutLock);
    if (alContext && alcMakeContextCurrent(nullptr) == ALC_TRUE)
        alcDestroyContext(alContext);

    if (alOutDev)
    {
        if (alcCloseDevice(alOutDev) == ALC_TRUE)
            alOutDev = nullptr;
        else
            qWarning() << "Failed to close output";
    }
}

void Audio::playMono16Sound(const QByteArray& data)
{
    QMutexLocker lock(audioOutLock);
    if (!alOutDev)
        return;

    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, data.data(), data.size(), 44100);
    alSourcef(alMainSource, AL_GAIN, outputVolume);
    alSourcei(alMainSource, AL_BUFFER, buffer);
    alSourcePlay(alMainSource);
    alDeleteBuffers(1, &buffer);
}
#include <QDebug>
void Audio::playGroupAudioQueued(Tox*,int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate, void* core)
{
    QMetaObject::invokeMethod(instance, "playGroupAudio", Qt::BlockingQueuedConnection,
                              Q_ARG(int,group), Q_ARG(int,peer), Q_ARG(const int16_t*,data),
                              Q_ARG(unsigned,samples), Q_ARG(uint8_t,channels), Q_ARG(unsigned,sample_rate));
    emit static_cast<Core*>(core)->groupPeerAudioPlaying(group, peer);
}

void Audio::playGroupAudio(int group, int peer, const int16_t* data,
                           unsigned samples, uint8_t channels, unsigned sample_rate)
{
    assert(QThread::currentThread() == audioThread);

    QMutexLocker lock(audioOutLock);

    ToxGroupCall& call = Core::groupCalls[group];

    if (!call.active || call.muteVol)
        return;

    if (!call.alSources.contains(peer))
    {
        alGenSources(1, &call.alSources[peer]);
        alSourcef(call.alSources[peer], AL_GAIN, outputVolume);
    }

    size_t volume = 0;//data[1];
    int bufsize = samples * 2 * channels;
    for (int i = 0; i < bufsize; ++i)
        volume += abs(data[i]);//std::max(volume, data[i]);

    emit groupAudioPlayed(group, peer, volume / static_cast<float>(bufsize));

    playAudioBuffer(call.alSources[peer], data, samples, channels, sample_rate);
}

void Audio::playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate)
{
    assert(channels == 1 || channels == 2);

    QMutexLocker lock(audioOutLock);

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
    else if (queued < 16)
    {
        alGenBuffers(1, &bufid);
    }
    else
    {
        qDebug() << "Dropped frame";
        return;
    }

    alBufferData(bufid, (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data,
                    samples * 2 * channels, sampleRate);
    alSourceQueueBuffers(alSource, 1, &bufid);

    ALint state;
    alGetSourcei(alSource, AL_SOURCE_STATE, &state);
    alSourcef(alSource, AL_GAIN, outputVolume);
    if (state != AL_PLAYING)
        alSourcePlay(alSource);
}

bool Audio::isInputReady()
{
    return (alInDev && userCount);
}

bool Audio::isOutputClosed()
{
    return (alOutDev);
}

bool Audio::tryCaptureSamples(uint8_t* buf, int framesize)
{
    QMutexLocker lock(audioInLock);

    ALint samples=0;
    alcGetIntegerv(Audio::alInDev, ALC_CAPTURE_SAMPLES, sizeof(samples), &samples);
    if (samples < framesize)
        return false;

    memset(buf, 0, framesize * 2 * av_DefaultSettings.audio_channels); // Avoid uninitialized values (Valgrind)
    alcCaptureSamples(Audio::alInDev, buf, framesize);
    return true;
}

#ifdef QTOX_FILTER_AUDIO
#include "audiofilterer.h"

/* include for compatibility with older versions of OpenAL */
#ifndef ALC_ALL_DEVICES_SPECIFIER
#include <AL/alext.h>
#endif

void Audio::getEchoesToFilter(AudioFilterer* filterer, int framesize)
{
#ifdef ALC_LOOPBACK_CAPTURE_SAMPLES
    ALint samples;
    alcGetIntegerv(Audio::alOutDev, ALC_LOOPBACK_CAPTURE_SAMPLES, sizeof(samples), &samples);
    if (samples >= framesize)
    {
        int16_t buf[framesize];
        alcCaptureSamplesLoopback(Audio::alOutDev, buf, framesize);
        filterer->passAudioOutput(buf, framesize);
        filterer->setEchoDelayMs(5); // This 5ms is configurable I believe
    }
#else
    Q_UNUSED(filterer);
    Q_UNUSED(framesize);
#endif
}
#endif
