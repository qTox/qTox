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
#include "src/persistence/settings.h"
#include "src/core/coreav.h"

#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include <QFile>

#include <cassert>

Audio* Audio::instance{nullptr};

/**
Returns the singleton's instance. Will construct on first call.
*/
Audio& Audio::getInstance()
{
    if (!instance)
    {
        instance = new Audio();
        instance->startAudioThread();
    }
    return *instance;
}

Audio::Audio()
    : audioThread(new QThread())
    , alOutDev(nullptr)
    , alInDev(nullptr)
    , mInputInitialized(false)
    , mOutputInitialized(false)
    , outputVolume(1.0)
    , inputVolume(1.0)
    , alMainSource(0)
    , alContext(nullptr)
{
    audioThread->setObjectName("qTox Audio");
    connect(audioThread, &QThread::finished, audioThread, &QThread::deleteLater);
}

Audio::~Audio()
{
    audioThread->exit();
    audioThread->wait();
    if (audioThread->isRunning())
        audioThread->terminate();
    cleanupInput();
    cleanupOutput();
}

/**
Start the audio thread for capture and playback.
*/
void Audio::startAudioThread()
{
    if (!audioThread->isRunning())
        audioThread->start();
    else
        qWarning("Audio thread already started -> ignored.");

    moveToThread(audioThread);
}

/**
Returns the current output volume, between 0 and 1
*/
qreal Audio::getOutputVolume()
{
    QMutexLocker locker(&mAudioLock);
    return outputVolume;
}

/**
The volume must be between 0 and 1
*/
void Audio::setOutputVolume(qreal volume)
{
    QMutexLocker locker(&mAudioLock);

    outputVolume = volume;
    alSourcef(alMainSource, AL_GAIN, outputVolume);

    for (const ToxGroupCall& call : CoreAV::groupCalls)
    {
        alSourcef(call.alSource, AL_GAIN, outputVolume);
    }

    for (const ToxFriendCall& call : CoreAV::calls)
    {
        alSourcef(call.alSource, AL_GAIN, outputVolume);
    }
}

/**
The volume must be between 0 and 2
*/
void Audio::setInputVolume(qreal volume)
{
    QMutexLocker locker(&mAudioLock);
    inputVolume = volume;
}

/**
@brief Subscribe to capture sound from the opened input device.

If the input device is not open, it will be opened before capturing.
*/
void Audio::subscribeInput(const void* inListener)
{
    QMutexLocker locker(&mAudioLock);

    if (!alInDev)
        initInput(Settings::getInstance().getInDev());

    if (!inputSubscriptions.contains(inListener)) {
        inputSubscriptions << inListener;
        qDebug() << "Subscribed to audio input device [" << inputSubscriptions.size() << "subscriptions ]";
    }
}

/**
@brief Unsubscribe from capturing from an opened input device.

If the input device has no more subscriptions, it will be closed.
*/
void Audio::unsubscribeInput(const void* inListener)
{
    QMutexLocker locker(&mAudioLock);

    if (inListener && inputSubscriptions.size())
    {
        inputSubscriptions.removeOne(inListener);
        qDebug() << "Unsubscribed from audio input device [" << inputSubscriptions.size() << "subscriptions left ]";
    }

    if (inputSubscriptions.isEmpty())
        cleanupInput();
}

void Audio::subscribeOutput(const void* outListener)
{
    QMutexLocker locker(&mAudioLock);

    if (!alOutDev)
        initOutput(Settings::getInstance().getOutDev());

    if (!outputSubscriptions.contains(outListener)) {
        outputSubscriptions << outListener;
        qDebug() << "Subscribed to audio output device [" << outputSubscriptions.size() << "subscriptions ]";
    }
}

void Audio::unsubscribeOutput(const void* outListener)
{
    QMutexLocker locker(&mAudioLock);

    if (outListener && outputSubscriptions.size())
    {
        outputSubscriptions.removeOne(outListener);
        qDebug() << "Unsubscribed from audio output device [" << outputSubscriptions.size() << " subscriptions left ]";
    }

    if (outputSubscriptions.isEmpty())
        cleanupOutput();
}

void Audio::initInput(const QString& inDevDescr)
{
    qDebug() << "Opening audio input" << inDevDescr;

    mInputInitialized = false;
    if (inDevDescr == "none")
        return;

    assert(!alInDev);

    /// TODO: Try to actually detect if our audio source is stereo
    int stereoFlag = AUDIO_CHANNELS == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    const uint32_t sampleRate = AUDIO_SAMPLE_RATE;
    const uint16_t frameDuration = AUDIO_FRAME_DURATION;
    const uint32_t chnls = AUDIO_CHANNELS;
    const ALCsizei bufSize = (frameDuration * sampleRate * 4) / 1000 * chnls;
    if (inDevDescr.isEmpty())
    {
        const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
        if (pDeviceList)
        {
            alInDev = alcCaptureOpenDevice(pDeviceList, sampleRate, stereoFlag, bufSize);
            int len = strlen(pDeviceList);
#ifdef Q_OS_WIN
            QString inDev = QString::fromUtf8(pDeviceList, len);
#else
            QString inDev = QString::fromLocal8Bit(pDeviceList, len);
#endif
            Settings::getInstance().setInDev(inDev);
        }
        else
        {
            alInDev = alcCaptureOpenDevice(nullptr, sampleRate, stereoFlag, bufSize);
        }
    }
    else
        alInDev = alcCaptureOpenDevice(inDevDescr.toStdString().c_str(),
                                       sampleRate, stereoFlag, bufSize);

    if (alInDev)
        qDebug() << "Opened audio input" << inDevDescr;
    else
        qWarning() << "Cannot open input audio device" << inDevDescr;

    Core* core = Core::getInstance();
    if (core)
        core->getAv()->resetCallSources(); // Force to regen each group call's sources

    // Restart the capture if necessary
    if (alInDev)
    {
        alcCaptureStart(alInDev);
    }
    else
    {
#if (FIX_SND_PCM_PREPARE_BUG)
        alcCaptureStart(alInDev);
#endif
    }

    mInputInitialized = true;
}

/**
@internal

Open an audio output device
*/
bool Audio::initOutput(const QString& outDevDescr)
{
    qDebug() << "Opening audio output" << outDevDescr;

    mOutputInitialized = false;
    if (outDevDescr == "none")
        return true;

    assert(!alOutDev);

    if (outDevDescr.isEmpty())
        {
            // Attempt to default to the first available audio device.
            const ALchar *pDeviceList;
            if (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
                pDeviceList = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
            else
                pDeviceList = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
            if (pDeviceList)
            {
                alOutDev = alcOpenDevice(pDeviceList);
                int len = strlen(pDeviceList);
  #ifdef Q_OS_WIN
                QString outDev = QString::fromUtf8(pDeviceList, len);
  #else
                QString outDev = QString::fromLocal8Bit(pDeviceList, len);
  #endif
                Settings::getInstance().setOutDev(outDev);
            }
            else
            {
                alOutDev = alcOpenDevice(nullptr);
            }
        }
    else
        alOutDev = alcOpenDevice(outDevDescr.toStdString().c_str());

    if (alOutDev)
    {
        alContext = alcCreateContext(alOutDev, nullptr);
        if (alcMakeContextCurrent(alContext))
        {
            alGenSources(1, &alMainSource);
        }
        else
        {
            qWarning() << "Cannot create output audio context";
            cleanupOutput();
            return false;
        }

        Core* core = Core::getInstance();
        if (core)
            core->getAv()->resetCallSources(); // Force to regen each group call's sources

        mOutputInitialized = true;
        return true;
    }

    qWarning() << "Cannot open output audio device" << outDevDescr;
    return false;
}

/**
Play a 44100Hz mono 16bit PCM sound
*/
void Audio::playMono16Sound(const QByteArray& data)
{
    QMutexLocker locker(&mAudioLock);

    if (!alOutDev)
        initOutput(Settings::getInstance().getOutDev());

    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, AL_FORMAT_MONO16, data.data(), data.size(), 44100);
    alSourcef(alMainSource, AL_GAIN, outputVolume);
    alSourcei(alMainSource, AL_BUFFER, buffer);
    alSourcePlay(alMainSource);

    ALint sizeInBytes;
    ALint channels;
    ALint bits;

    alGetBufferi(buffer, AL_SIZE, &sizeInBytes);
    alGetBufferi(buffer, AL_CHANNELS, &channels);
    alGetBufferi(buffer, AL_BITS, &bits);

    ALint frequency;
    alGetBufferi(buffer, AL_FREQUENCY, &frequency);

    alDeleteBuffers(1, &buffer);

    if (outputSubscriptions.isEmpty())
        cleanupOutput();
}

/**
Play a 44100Hz mono 16bit PCM sound from a file
*/
void Audio::playMono16Sound(const char *path)
{
    QFile sndFile(path);
    sndFile.open(QIODevice::ReadOnly);
    playMono16Sound(sndFile.readAll());
}

/**
@brief May be called from any thread, will always queue a call to playGroupAudio.

The first and last argument are ignored, but allow direct compatibility with toxcore.
*/
void Audio::playGroupAudioQueued(void*,int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate, void* core)
{
    QMetaObject::invokeMethod(instance, "playGroupAudio", Qt::BlockingQueuedConnection,
                              Q_ARG(int,group), Q_ARG(int,peer), Q_ARG(const int16_t*,data),
                              Q_ARG(unsigned,samples), Q_ARG(uint8_t,channels), Q_ARG(unsigned,sample_rate));
    emit static_cast<Core*>(core)->groupPeerAudioPlaying(group, peer);
}

/**
Must be called from the audio thread, plays a group call's received audio
*/
void Audio::playGroupAudio(int group, int peer, const int16_t* data,
                           unsigned samples, uint8_t channels, unsigned sample_rate)
{
    assert(QThread::currentThread() == audioThread);
    QMutexLocker locker(&mAudioLock);

    if (!CoreAV::groupCalls.contains(group))
        return;

    ToxGroupCall& call = CoreAV::groupCalls[group];

    if (call.inactive || call.muteVol)
        return;

    if (!call.alSource)
    {
        alGenSources(1, &call.alSource);
        alSourcef(call.alSource, AL_GAIN, outputVolume);
    }

    qreal volume = 0.;
    int bufsize = samples * 2 * channels;
    for (int i = 0; i < bufsize; ++i)
        volume += abs(data[i]);

    emit groupAudioPlayed(group, peer, volume / bufsize);

    playAudioBuffer(call.alSource, data, samples, channels, sample_rate);
}

void Audio::playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate)
{
    assert(channels == 1 || channels == 2);

    QMutexLocker locker(&getInstance().mAudioLock);

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
        return;
    }

    alBufferData(bufid, (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data,
                    samples * 2 * channels, sampleRate);
    alSourceQueueBuffers(alSource, 1, &bufid);

    ALint state;
    alGetSourcei(alSource, AL_SOURCE_STATE, &state);
    alSourcef(alSource, AL_GAIN, getInstance().outputVolume);
    if (state != AL_PLAYING)
        alSourcePlay(alSource);
}

/**
@internal

Close active audio input device.
*/
void Audio::cleanupInput()
{
    mInputInitialized = false;

    if (alInDev)
    {
#if (!FIX_SND_PCM_PREPARE_BUG)
        qDebug() << "stopping audio capture";
        alcCaptureStop(alInDev);
#endif

        qDebug() << "Closing audio input";
        if (alcCaptureCloseDevice(alInDev) == ALC_TRUE)
            alInDev = nullptr;
        else
            qWarning() << "Failed to close input";
    }
}

/**
@internal

Close active audio output device
*/
void Audio::cleanupOutput()
{
    mOutputInitialized = false;

    if (alOutDev) {
        qDebug() << "Closing audio output";
        alSourcei(alMainSource, AL_LOOPING, AL_FALSE);
        alSourceStop(alMainSource);
        alDeleteSources(1, &alMainSource);

        if (!alcMakeContextCurrent(nullptr))
            qWarning("Failed to clear current audio context.");

        alcDestroyContext(alContext);
        alContext = nullptr;

        if (alcCloseDevice(alOutDev))
            alOutDev = nullptr;
        else
            qWarning("Failed to close output.");
    }
}

/**
Returns true if the input device is open and suscribed to
*/
bool Audio::isInputReady()
{
    QMutexLocker locker(&mAudioLock);
    return alInDev && mInputInitialized;
}

/**
Returns true if the output device is open
*/
bool Audio::isOutputReady()
{
    QMutexLocker locker(&mAudioLock);
    return alOutDev && mOutputInitialized;
}

void Audio::createSource(ALuint* source)
{
    alGenSources(1, source);
    alSourcef(*source, AL_GAIN, getInstance().outputVolume);
}

void Audio::deleteSource(ALuint* source)
{
    if (alIsSource(*source))
        alDeleteSources(1, source);
    else
        qWarning() << "Trying to delete invalid audio source"<<*source;
}

void Audio::startLoop()
{
    alSourcei(alMainSource, AL_LOOPING, AL_TRUE);
}

void Audio::stopLoop()
{
    alSourcei(alMainSource, AL_LOOPING, AL_FALSE);
    alSourceStop(alMainSource);
}

/**
Does nothing and return false on failure
*/
bool Audio::tryCaptureSamples(int16_t* buf, int samples)
{
    QMutexLocker lock(&mAudioLock);

    if (!(alInDev && mInputInitialized))
        return false;

    ALint curSamples=0;
    alcGetIntegerv(alInDev, ALC_CAPTURE_SAMPLES, sizeof(curSamples), &curSamples);
    if (curSamples < samples)
        return false;

    alcCaptureSamples(alInDev, buf, samples);

    for (size_t i = 0; i < samples * AUDIO_CHANNELS; ++i)
    {
        int sample = buf[i] * pow(inputVolume, 2);

        if (sample < std::numeric_limits<int16_t>::min())
            sample = std::numeric_limits<int16_t>::min();
        else if (sample > std::numeric_limits<int16_t>::max())
            sample = std::numeric_limits<int16_t>::max();

        buf[i] = sample;
    }

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
    alcGetIntegerv(Audio::getInstance().alOutDev, ALC_LOOPBACK_CAPTURE_SAMPLES, sizeof(samples), &samples);
    if (samples >= framesize)
    {
        int16_t buf[framesize];
        alcCaptureSamplesLoopback(Audio::getInstance().alOutDev, buf, framesize);
        filterer->passAudioOutput(buf, framesize);
        filterer->setEchoDelayMs(5); // This 5ms is configurable I believe
    }
#else
    Q_UNUSED(filterer);
    Q_UNUSED(framesize);
#endif
}
#endif
