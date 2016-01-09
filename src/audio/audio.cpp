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
#include <QFile>
#include <QMutexLocker>
#include <QPointer>
#include <QThread>

#include <cassert>

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/al.h>
 #include <AL/alc.h>
#endif

#ifndef ALC_ALL_DEVICES_SPECIFIER
// compatibility with older versions of OpenAL
#include <AL/alext.h>
#endif

#ifdef QTOX_FILTER_AUDIO
#include "audiofilterer.h"
#endif

/**
@class AudioPlayer

@brief Non-blocking audio player.

The audio data is played from start to finish (no streaming).
*/
class AudioPlayer : public QThread
{
public:
    AudioPlayer(ALuint source, const QByteArray& data)
        : mSource(source)
    {
        alGenBuffers(1, &mBuffer);
        alBufferData(mBuffer, AL_FORMAT_MONO16, data.constData(), data.size(), 44100);
        alSourcei(mSource, AL_BUFFER, mBuffer);

        connect(this, &AudioPlayer::finished, this, &AudioPlayer::deleteLater);
    }

private:
    void run() override final
    {
        alSourceRewind(mSource);
        alSourcePlay(mSource);

        QMutexLocker locker(&playLock);
        ALint state = AL_PLAYING;
        while (state == AL_PLAYING) {
            alGetSourcei(mSource, AL_SOURCE_STATE, &state);
            waitPlaying.wait(&playLock, 2000);
        }

        alSourceStop(mSource);
        alDeleteBuffers(1, &mBuffer);
    }

public:
    QMutex playLock;
    QWaitCondition waitPlaying;

private:
    ALuint      mBuffer;
    ALuint      mSource;
};

class AudioMeter : public QThread
{
public:
    AudioMeter()
    {
        connect(this, &AudioMeter::finished, this, &AudioMeter::deleteLater);
    }

    inline void stop()
    {
        requestInterruption();
    }

private:
    void run() override final
    {
        static const int framesize = AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS;

        Audio& audio = Audio::getInstance();

        mNewMaxGain = 0.f;

        QMutexLocker locker(&mMeterLock);
        while (!isInterruptionRequested()) {
            mListenerReady.wait(locker.mutex());
            locker.relock();

            int16_t buff[framesize] = {0};
            if (audio.tryCaptureSamples(buff, AUDIO_FRAME_SAMPLE_COUNT)) {
                mNewMaxGain = 0.f;
                for (int i = 0; i < framesize; ++i) {
                    mNewMaxGain = qMax(mNewMaxGain, qAbs(buff[i]) / 32767.0);
                }
            }

            mGainMeasured.wakeAll();
        }
    }

public:
    QMutex          mMeterLock;
    QWaitCondition  mListenerReady;
    QWaitCondition  mGainMeasured;
    qreal           mNewMaxGain;
};

class AudioPrivate
{
    typedef QList<Audio::SID> ALSources;

public:
    AudioPrivate()
        : audioThread(new QThread)
        , alInDev(nullptr)
        , alOutDev(nullptr)
        , alContext(nullptr)
        , inputVolume(1.f)
        , outputVolume(1.f)
        , inputInitialized(false)
        , outputInitialized(false)
        , inSubscriptions(0)
    {
        audioThread->setObjectName("qTox Audio");
        QObject::connect(audioThread, &QThread::finished, audioThread, &QThread::deleteLater);
    }

    ~AudioPrivate()
    {
        if (audioMeter)
            audioMeter->stop();

        audioThread->exit();
        audioThread->wait();
        cleanupInput();
        cleanupOutput();
    }

    bool initInput(QString inDevDescr);
    bool initOutput(QString outDevDescr);
    void cleanupInput();
    void cleanupOutput();

public:
    QThread*            audioThread;
    QMutex              audioLock;

    ALCdevice*          alInDev;
    ALCdevice*          alOutDev;
    ALCcontext*         alContext;
    ALuint              alMainSource;
    qreal               inputVolume;
    qreal               outputVolume;
    bool                inputInitialized;
    bool                outputInitialized;

    quint32             inSubscriptions;
    ALSources           outSources;

    QPointer<AudioMeter>    audioMeter;
};

AudioMeterListener::AudioMeterListener(AudioMeter* measureThread)
    : mActive(false)
    , mAudioMeter(measureThread)
{
    assert(mAudioMeter);
}

void AudioMeterListener::start()
{
    if (!mAudioMeter->isRunning()) {
        mAudioMeter->start();
        // TODO: ensure that audiometer is running
        //       -> Start listeners from AudioMeter::started signal
        while (!mAudioMeter->isRunning())
            QThread::msleep(10);
    }

    QThread* listener = new QThread;
    connect(listener, &QThread::started, this, &AudioMeterListener::doListen);
    connect(listener, &QThread::finished, listener, &QThread::deleteLater);
    moveToThread(listener);

    listener->start();
}

void AudioMeterListener::stop()
{
    mActive = false;
}

void AudioMeterListener::processed()
{
    mGainProcessed.wakeAll();
}

void AudioMeterListener::doListen()
{
    mMaxGain = 0.f;
    mActive = true;

    QMutexLocker locker(&mAudioMeter->mMeterLock);
    while (mActive) {
        mAudioMeter->mListenerReady.wakeAll();
        mAudioMeter->mGainMeasured.wait(&mAudioMeter->mMeterLock);
        locker.relock();

        if (mAudioMeter->mNewMaxGain > mMaxGain) {
            mMaxGain = mAudioMeter->mNewMaxGain;
            emit gainChanged(mMaxGain);
        } else if (mMaxGain > 0.02f) {
            mMaxGain -= 0.008f;
            emit gainChanged(mMaxGain);
        }

        mGainProcessed.wait(locker.mutex(), 10);
        locker.relock();
    }

    mAudioMeter->requestInterruption();
}

/**
Returns the singleton instance.
*/
Audio& Audio::getInstance()
{
    static Audio instance;
    return instance;
}

AudioMeterListener* Audio::createAudioMeterListener() const
{
    if (!d->audioMeter)
        d->audioMeter = new AudioMeter;

    return new AudioMeterListener(d->audioMeter);
}

Audio::Audio()
    : d(new AudioPrivate)
{
    moveToThread(d->audioThread);

    if (!d->audioThread->isRunning())
        d->audioThread->start();
    else
        qWarning("Audio thread already started -> ignored.");
}

Audio::~Audio()
{
    delete d;
}

/**
Returns the current output volume, between 0 and 1
*/
qreal Audio::outputVolume()
{
    QMutexLocker locker(&d->audioLock);
    return d->outputVolume;
}

/**
The volume must be between 0 and 1
*/
void Audio::setOutputVolume(qreal volume)
{
    QMutexLocker locker(&d->audioLock);

    d->outputVolume = volume;
    alSourcef(d->alMainSource, AL_GAIN, volume);

    for (const ToxGroupCall& call : CoreAV::groupCalls)
    {
        alSourcef(call.alSource, AL_GAIN, volume);
    }

    for (const ToxFriendCall& call : CoreAV::calls)
    {
        alSourcef(call.alSource, AL_GAIN, volume);
    }
}

qreal Audio::inputVolume()
{
    QMutexLocker locker(&d->audioLock);

    return d->inputVolume;
}

void Audio::setInputVolume(qreal volume)
{
    QMutexLocker locker(&d->audioLock);
    d->inputVolume = volume;
}

void Audio::reinitInput(const QString& inDevDesc)
{
    QMutexLocker locker(&d->audioLock);
    d->cleanupInput();
    d->initInput(inDevDesc);
}

bool Audio::reinitOutput(const QString& outDevDesc)
{
    QMutexLocker locker(&d->audioLock);
    d->cleanupOutput();
    return d->initOutput(outDevDesc);
}

/**
@brief Subscribe to capture sound from the opened input device.

If the input device is not open, it will be opened before capturing.
*/
void Audio::subscribeInput()
{
    QMutexLocker locker(&d->audioLock);

    if (!d->alInDev) {
        if (!d->initInput(Settings::getInstance().getInDev())) {
            qWarning("Failed to subscribe to audio input device.");
            return;
        }
    }

    d->inSubscriptions++;
    qDebug() << "Subscribed to audio input device [" << d->inSubscriptions << "subscriptions ]";
}

/**
@brief Unsubscribe from capturing from an opened input device.

If the input device has no more subscriptions, it will be closed.
*/
void Audio::unsubscribeInput()
{
    QMutexLocker locker(&d->audioLock);

    if (d->inSubscriptions > 0) {
        d->inSubscriptions--;
        qDebug() << "Unsubscribed from audio input device [" << d->inSubscriptions << "subscriptions left ]";
    }

    if (!d->inSubscriptions)
        d->cleanupInput();
}

bool AudioPrivate::initInput(QString inDevDescr)
{
    qDebug() << "Opening audio input" << inDevDescr;

    inputInitialized = false;
    if (inDevDescr == "none")
        return true;

    assert(!alInDev);

    /// TODO: Try to actually detect if our audio source is stereo
    int stereoFlag = AUDIO_CHANNELS == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    const uint32_t sampleRate = AUDIO_SAMPLE_RATE;
    const uint16_t frameDuration = AUDIO_FRAME_DURATION;
    const uint32_t chnls = AUDIO_CHANNELS;
    const ALCsizei bufSize = (frameDuration * sampleRate * 4) / 1000 * chnls;
    if (inDevDescr.isEmpty()) {
        const ALchar *pDeviceList = Audio::inDeviceNames();
        if (pDeviceList)
            inDevDescr = QString::fromUtf8(pDeviceList, strlen(pDeviceList));
    }

    if (!inDevDescr.isEmpty())
        alInDev = alcCaptureOpenDevice(inDevDescr.toUtf8().constData(),
                                       sampleRate, stereoFlag, bufSize);

    // Restart the capture if necessary
    if (alInDev) {
        qDebug() << "Opened audio input" << inDevDescr;
        alcCaptureStart(alInDev);
    } else {
        qCritical() << "Failed to initialize audio input device:" << inDevDescr;
        return false;
    }

    inputInitialized = true;
    return true;
}

/**
@internal

Open an audio output device
*/
bool AudioPrivate::initOutput(QString outDevDescr)
{
    qDebug() << "Opening audio output" << outDevDescr;
    outSources.clear();

    outputInitialized = false;
    if (outDevDescr == "none")
        return true;

    assert(!alOutDev);

    if (outDevDescr.isEmpty()) {
        // default to the first available audio device.
        const ALchar *pDeviceList = Audio::outDeviceNames();
        if (pDeviceList)
            outDevDescr = QString::fromUtf8(pDeviceList, strlen(pDeviceList));
    }

    if (!outDevDescr.isEmpty())
        alOutDev = alcOpenDevice(outDevDescr.toUtf8().constData());

    if (alOutDev)
    {
        alContext = alcCreateContext(alOutDev, nullptr);
        if (alcMakeContextCurrent(alContext))
        {
            alGenSources(1, &alMainSource);
            alSourcef(alMainSource, AL_GAIN, outputVolume);
        }
        else
        {
            qWarning() << "Cannot create output audio context";
            cleanupOutput();
            return false;
        }

        Core* core = Core::getInstance();
        if (core)
            core->getAv()->invalidateCallSources(); // Force to regen each group call's sources

        outputInitialized = true;
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
    QMutexLocker locker(&d->audioLock);

    if (!d->alOutDev)
        d->initOutput(Settings::getInstance().getOutDev());

    alSourcef(d->alMainSource, AL_GAIN, d->outputVolume);

    AudioPlayer *player = new AudioPlayer(d->alMainSource, data);
    connect(player, &AudioPlayer::finished, [=]() {
        QMutexLocker locker(&d->audioLock);

        if (d->outSources.isEmpty())
            d->cleanupOutput();
        else
            qDebug("Audio output not closed -> there are pending subscriptions.");
    });

    player->start();
}

/**
Play a 44100Hz mono 16bit PCM sound from a file
*/
void Audio::playMono16Sound(const QString& path)
{
    QFile sndFile(path);
    sndFile.open(QIODevice::ReadOnly);
    playMono16Sound(QByteArray(sndFile.readAll()));
}

/**
@brief May be called from any thread, will always queue a call to playGroupAudio.

The first and last argument are ignored, but allow direct compatibility with toxcore.
*/
void Audio::playGroupAudioQueued(void*,int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate, void* core)
{
    QMetaObject::invokeMethod(&Audio::getInstance(), "playGroupAudio", Qt::BlockingQueuedConnection,
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
    assert(QThread::currentThread() == d->audioThread);
    QMutexLocker locker(&d->audioLock);

    if (!CoreAV::groupCalls.contains(group))
        return;

    ToxGroupCall& call = CoreAV::groupCalls[group];

    if (call.inactive || call.muteVol)
        return;

    if (!call.alSource)
    {
        alGenSources(1, &call.alSource);
        alSourcef(call.alSource, AL_GAIN, d->outputVolume);
    }

    qreal volume = 0.;
    int bufsize = samples * 2 * channels;
    for (int i = 0; i < bufsize; ++i)
        volume += abs(data[i]);

    emit groupAudioPlayed(group, peer, volume / bufsize);

    playAudioBuffer(call.alSource, data, samples, channels, sample_rate);
}

void Audio::playAudioBuffer(quint32 alSource, const int16_t *data, int samples, unsigned channels, int sampleRate)
{
    assert(channels == 1 || channels == 2);
    QMutexLocker locker(&d->audioLock);

    if (!(d->alOutDev && d->outputInitialized))
        return;

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
    alSourcef(alSource, AL_GAIN, d->outputVolume);
    if (state != AL_PLAYING)
        alSourcePlay(alSource);
}

/**
@internal

Close active audio input device.
*/
void AudioPrivate::cleanupInput()
{
    inputInitialized = false;

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
void AudioPrivate::cleanupOutput()
{
    outputInitialized = false;

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
    QMutexLocker locker(&d->audioLock);
    return d->alInDev && d->inputInitialized;
}

/**
Returns true if the output device is open
*/
bool Audio::isOutputReady()
{
    QMutexLocker locker(&d->audioLock);
    return d->alOutDev && d->outputInitialized;
}

const char* Audio::outDeviceNames()
{
    const char* pDeviceList;
    if (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
        pDeviceList = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    else
        pDeviceList = alcGetString(NULL, ALC_DEVICE_SPECIFIER);

    return pDeviceList;
}

const char* Audio::inDeviceNames()
{
    return alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
}

void Audio::subscribeOutput(SID& sid)
{
    QMutexLocker locker(&d->audioLock);

    if (!d->alOutDev) {
        if (!d->initOutput(Settings::getInstance().getOutDev())) {
            qWarning("Failed to subscribe to audio output device.");
            return;
        }
    }

    alGenSources(1, &sid);
    assert(sid);
    alSourcef(sid, AL_GAIN, 1.f);
    d->outSources << sid;
    qDebug() << "Audio source" << sid << "created. Sources active:"
             << d->outSources.size();
}

void Audio::unsubscribeOutput(SID& sid)
{
    QMutexLocker locker(&d->audioLock);

    d->outSources.removeAll(sid);

    if (sid) {
        if (alIsSource(sid)) {
            alDeleteSources(1, &sid);
            qDebug() << "Audio source" << sid << "deleted. Sources active:"
                     << d->outSources.size();
        } else {
            qWarning() << "Trying to delete invalid audio source" << sid;
        }

        sid = 0;
    }

    if (d->outSources.isEmpty())
        d->cleanupOutput();
}

void Audio::startLoop()
{
    QMutexLocker locker(&d->audioLock);
    alSourcei(d->alMainSource, AL_LOOPING, AL_TRUE);
}

void Audio::stopLoop()
{
    QMutexLocker locker(&d->audioLock);
    alSourcei(d->alMainSource, AL_LOOPING, AL_FALSE);
    alSourceStop(d->alMainSource);
}

/**
Does nothing and return false on failure
*/
bool Audio::tryCaptureSamples(int16_t* buf, int samples)
{
    QMutexLocker lock(&d->audioLock);

    if (!(d->alInDev && d->inputInitialized))
        return false;

    ALint curSamples = 0;
    alcGetIntegerv(d->alInDev, ALC_CAPTURE_SAMPLES, sizeof(curSamples), &curSamples);
    if (curSamples < samples)
        return false;

    alcCaptureSamples(d->alInDev, buf, samples);

    for (size_t i = 0; i < samples * AUDIO_CHANNELS; ++i)
    {
        int sample = buf[i] * pow(d->inputVolume, 2);

        if (sample < std::numeric_limits<int16_t>::min())
            sample = std::numeric_limits<int16_t>::min();
        else if (sample > std::numeric_limits<int16_t>::max())
            sample = std::numeric_limits<int16_t>::max();

        buf[i] = sample;
    }

    return true;
}

#if defined(QTOX_FILTER_AUDIO) && defined(ALC_LOOPBACK_CAPTURE_SAMPLES)
void Audio::getEchoesToFilter(AudioFilterer* filterer, int samples)
{
    QMutexLocker locker(&d->audioLock);

    ALint samples;
    alcGetIntegerv(&d->alOutDev, ALC_LOOPBACK_CAPTURE_SAMPLES, sizeof(samples), &samples);
    if (samples >= samples)
    {
        int16_t buf[samples];
        alcCaptureSamplesLoopback(&d->alOutDev, buf, samples);
        filterer->passAudioOutput(buf, samples);
        filterer->setEchoDelayMs(5); // This 5ms is configurable I believe
    }
}
#endif
