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
#include "src/core/coreav.h"
#include "src/persistence/settings.h"

#include <QDebug>
#include <QFile>
#include <QMutexLocker>
#include <QPointer>
#include <QThread>
#include <QWaitCondition>

#include <cassert>

#ifdef QTOX_FILTER_AUDIO
#include "audiofilterer.h"
#endif

#ifndef CHECK_AL_ERROR
#define CHECK_AL_ERROR \
    do { \
    const ALenum al_err = alGetError(); \
    if (al_err) { qWarning("OpenAL error: %d", al_err); } \
    } while (0)
#endif

#ifndef CHECK_ALC_ERROR
#define CHECK_ALC_ERROR(device) \
    do { \
    const ALCenum alc_err = alcGetError(device); \
    if (alc_err) { qWarning("OpenAL error: %d", alc_err); } \
    } while (0)
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

/**
Returns the singleton instance.
*/
Audio& Audio::getInstance()
{
    static Audio instance;
    return instance;
}

Audio::Audio()
    : audioThread(new QThread)
    , alInDev(nullptr)
    , inputInitialized(false)
    , inSubscriptions(0)
    , alOutDev(nullptr)
    , alOutContext(nullptr)
    , alMainSource(0)
    , outputInitialized(false)
{
    audioThread->setObjectName("qTox Audio");
    QObject::connect(audioThread, &QThread::finished, audioThread, &QThread::deleteLater);

    moveToThread(audioThread);

    if (!audioThread->isRunning())
        audioThread->start();
    else
        qWarning("Audio thread already started -> ignored.");
}

Audio::~Audio()
{
    audioThread->exit();
    audioThread->wait();
    cleanupInput();
    cleanupOutput();
}

/**
Returns the current output volume (between 0 and 1)
*/
qreal Audio::outputVolume()
{
    QMutexLocker locker(&audioLock);

    ALfloat volume = 0.0;

    if (alOutDev) {
        alGetListenerf(AL_GAIN, &volume);
        CHECK_AL_ERROR;
    }

    return volume;
}

/**
Set the master output volume.

@param[in] volume   the master volume (between 0 and 1)
*/
void Audio::setOutputVolume(qreal volume)
{
    QMutexLocker locker(&audioLock);

    if (volume < 0.0) {
        volume = 0.0;
    } else if (volume > 1.0) {
        volume = 1.0;
    }

    alListenerf(AL_GAIN, volume);
    CHECK_AL_ERROR;
}

qreal Audio::inputVolume()
{
    QMutexLocker locker(&audioLock);

    return gain;
}

void Audio::setInputVolume(qreal volume)
{
    QMutexLocker locker(&audioLock);

    if (volume < 0.0) {
        volume = 0.0;
    } else if (volume > 1.0) {
        volume = 1.0;
    }

    gain = volume;
}

void Audio::reinitInput(const QString& inDevDesc)
{
    QMutexLocker locker(&audioLock);
    cleanupInput();
    initInput(inDevDesc);
}

bool Audio::reinitOutput(const QString& outDevDesc)
{
    QMutexLocker locker(&audioLock);
    cleanupOutput();
    return initOutput(outDevDesc);
}

/**
@brief Subscribe to capture sound from the opened input device.

If the input device is not open, it will be opened before capturing.
*/
void Audio::subscribeInput()
{
    QMutexLocker locker(&audioLock);

    if (!autoInitInput()) {
        qWarning("Failed to subscribe to audio input device.");
        return;
    }

    inSubscriptions++;
    qDebug() << "Subscribed to audio input device [" << inSubscriptions << "subscriptions ]";
}

/**
@brief Unsubscribe from capturing from an opened input device.

If the input device has no more subscriptions, it will be closed.
*/
void Audio::unsubscribeInput()
{
    QMutexLocker locker(&audioLock);

    if (inSubscriptions > 0) {
        inSubscriptions--;
        qDebug() << "Unsubscribed from audio input device [" << inSubscriptions << "subscriptions left ]";
    }

    if (!inSubscriptions)
        cleanupInput();
}

/**
Initialize audio input device, if not initialized.

@return true, if device was initialized; false otherwise
*/
bool Audio::autoInitInput()
{
    return alInDev ? true : initInput(Settings::getInstance().getInDev());
}

/**
Initialize audio output device, if not initialized.

@return true, if device was initialized; false otherwise
*/
bool Audio::autoInitOutput()
{
    return alOutDev ? true : initOutput(Settings::getInstance().getOutDev());
}

bool Audio::initInput(QString inDevDescr)
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
    if (!alInDev) {
        qWarning() << "Failed to initialize audio input device:" << inDevDescr;
        return false;
    }

    qDebug() << "Opened audio input" << inDevDescr;
    alcCaptureStart(alInDev);

    inputInitialized = true;
    return true;
}

/**
@internal

Open an audio output device
*/
bool Audio::initOutput(QString outDevDescr)
{
    qDebug() << "Opening audio output" << outDevDescr;
    outSources.clear();

    outputInitialized = false;
    if (outDevDescr == "none")
        return false;

    assert(!alOutDev);

    if (outDevDescr.isEmpty()) {
        // default to the first available audio device.
        const ALchar *pDeviceList = Audio::outDeviceNames();
        if (pDeviceList)
            outDevDescr = QString::fromUtf8(pDeviceList, strlen(pDeviceList));
    }

    if (!outDevDescr.isEmpty())
        alOutDev = alcOpenDevice(outDevDescr.toUtf8().constData());

    if (!alOutDev)
    {
        qWarning() << "Cannot open output audio device" << outDevDescr;
        return false;
    }

    qDebug() << "Opened audio output" << outDevDescr;
    alOutContext = alcCreateContext(alOutDev, nullptr);
    CHECK_ALC_ERROR(alOutDev);

    if (!alcMakeContextCurrent(alOutContext)) {
        qWarning() << "Cannot create output audio context";
        return false;
    }

    alGenSources(1, &alMainSource); CHECK_AL_ERROR;
    alSourcef(alMainSource, AL_GAIN, 1.f); CHECK_AL_ERROR;
    alSourcef(alMainSource, AL_PITCH, 1.f); CHECK_AL_ERROR;
    alSource3f(alMainSource, AL_VELOCITY, 0.f, 0.f, 0.f); CHECK_AL_ERROR;
    alSource3f(alMainSource, AL_POSITION, 0.f, 0.f, 0.f); CHECK_AL_ERROR;

    // init master volume
    alListenerf(AL_GAIN, Settings::getInstance().getOutVolume() * 0.01f);
    CHECK_AL_ERROR;

    Core* core = Core::getInstance();
    if (core) {
        // reset each call's audio source
        core->getAv()->invalidateCallSources();
    }

    outputInitialized = true;
    return true;
}

/**
Play a 44100Hz mono 16bit PCM sound
*/
void Audio::playMono16Sound(const QByteArray& data)
{
    QMutexLocker locker(&audioLock);

    if (!autoInitOutput())
        return;

    AudioPlayer *player = new AudioPlayer(alMainSource, data);
    connect(player, &AudioPlayer::finished, [=]() {
        QMutexLocker locker(&audioLock);

        if (outSources.isEmpty())
            cleanupOutput();
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
    assert(QThread::currentThread() == audioThread);
    QMutexLocker locker(&audioLock);

    if (!CoreAV::groupCalls.contains(group))
        return;

    ToxGroupCall& call = CoreAV::groupCalls[group];

    if (call.inactive || call.muteVol)
        return;

    qreal volume = 0.;
    int bufsize = samples * 2 * channels;
    for (int i = 0; i < bufsize; ++i)
        volume += abs(data[i]);

    emit groupAudioPlayed(group, peer, volume / bufsize);

    locker.unlock();

    playAudioBuffer(call.alSource, data, samples, channels, sample_rate);
}

void Audio::playAudioBuffer(quint32 alSource, const int16_t *data, int samples, unsigned channels, int sampleRate)
{
    assert(channels == 1 || channels == 2);
    QMutexLocker locker(&audioLock);

    if (!(alOutDev && outputInitialized))
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
    if (state != AL_PLAYING)
        alSourcePlay(alSource);
}

/**
@internal

Close active audio input device.
*/
void Audio::cleanupInput()
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
void Audio::cleanupOutput()
{
    outputInitialized = false;

    if (alOutDev) {
        alSourcei(alMainSource, AL_LOOPING, AL_FALSE);
        alSourceStop(alMainSource);
        alDeleteSources(1, &alMainSource);

        if (!alcMakeContextCurrent(nullptr))
            qWarning("Failed to clear audio context.");

        alcDestroyContext(alOutContext);
        alOutContext = nullptr;

        qDebug() << "Closing audio output";
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
    QMutexLocker locker(&audioLock);
    return alInDev && inputInitialized;
}

/**
Returns true if the output device is open
*/
bool Audio::isOutputReady()
{
    QMutexLocker locker(&audioLock);
    return alOutDev && outputInitialized;
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

void Audio::subscribeOutput(ALuint& sid)
{
    QMutexLocker locker(&audioLock);

    if (!autoInitOutput()) {
        qWarning("Failed to subscribe to audio output device.");
        return;
    }

    if (!alcMakeContextCurrent(alOutContext)) {
        qWarning("Failed to activate output context.");
        return;
    }

    alGenSources(1, &sid);
    assert(sid);
    outSources << sid;

    // initialize source
    alSourcef(sid, AL_GAIN, 1.f); CHECK_AL_ERROR;
    alSourcef(sid, AL_PITCH, 1.f); CHECK_AL_ERROR;
    alSource3f(sid, AL_VELOCITY, 0.f, 0.f, 0.f); CHECK_AL_ERROR;
    alSource3f(sid, AL_POSITION, 0.f, 0.f, 0.f); CHECK_AL_ERROR;

    qDebug() << "Audio source" << sid << "created. Sources active:"
             << outSources.size();
}

void Audio::unsubscribeOutput(ALuint &sid)
{
    QMutexLocker locker(&audioLock);

    outSources.removeAll(sid);

    if (sid) {
        if (alIsSource(sid)) {
            alDeleteSources(1, &sid);
            qDebug() << "Audio source" << sid << "deleted. Sources active:"
                     << outSources.size();
        } else {
            qWarning() << "Trying to delete invalid audio source" << sid;
        }

        sid = 0;
    }

    if (outSources.isEmpty())
        cleanupOutput();
}

void Audio::startLoop()
{
    QMutexLocker locker(&audioLock);
    alSourcei(alMainSource, AL_LOOPING, AL_TRUE);
}

void Audio::stopLoop()
{
    QMutexLocker locker(&audioLock);
    alSourcei(alMainSource, AL_LOOPING, AL_FALSE);
    alSourceStop(alMainSource);
}

/**
Does nothing and return false on failure
*/
bool Audio::tryCaptureSamples(int16_t* buf, int samples)
{
    QMutexLocker lock(&audioLock);

    if (!(alInDev && inputInitialized))
        return false;

    ALint curSamples = 0;
    alcGetIntegerv(alInDev, ALC_CAPTURE_SAMPLES, sizeof(curSamples), &curSamples);
    if (curSamples < samples)
        return false;

    alcCaptureSamples(alInDev, buf, samples);

    for (size_t i = 0; i < samples * AUDIO_CHANNELS; ++i)
    {
        int sample = buf[i];

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
    QMutexLocker locker(&audioLock);

    ALint samples;
    alcGetIntegerv(&alOutDev, ALC_LOOPBACK_CAPTURE_SAMPLES, sizeof(samples), &samples);
    if (samples >= samples)
    {
        int16_t buf[samples];
        alcCaptureSamplesLoopback(&alOutDev, buf, samples);
        filterer->passAudioOutput(buf, samples);
        filterer->setEchoDelayMs(5); // This 5ms is configurable I believe
    }
}
#endif
