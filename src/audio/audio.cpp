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

#include "audio.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/persistence/settings.h"

#include <QDebug>
#include <QFile>
#include <QMutexLocker>
#include <QPointer>
#include <QThread>
#include <QtMath>
#include <QWaitCondition>

#include <cassert>

/**
 * @class Audio::Private
 *
 * @brief Encapsulates private audio framework from public qTox Audio API.
 */
class Audio::Private
{
public:
    Private()
        : minInputGain{-30.0}
        , maxInputGain{30.0}
        , gain{0.0}
        , gainFactor{0.0}
    {
    }

    static const ALchar* inDeviceNames()
    {
        return alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    }

    static const ALchar* outDeviceNames()
    {
        return (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
                ? alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER)
                : alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    }

    qreal inputGain() const
    {
        return gain;
    }

    qreal inputGainFactor() const
    {
        return gainFactor;
    }

    void setInputGain(qreal dB)
    {
        gain = qBound(minInputGain, dB, maxInputGain);
        gainFactor = qPow(10.0, (gain / 20.0));
    }

public:
    qreal   minInputGain;
    qreal   maxInputGain;

private:
    qreal   gain;
    qreal   gainFactor;
};

/**
 * @class Audio
 *
 * @fn void Audio::frameAvailable(const int16_t *pcm, size_t sample_count, uint8_t channels, uint32_t sampling_rate);
 *
 * When there are input subscribers, we regularly emit captured audio frames with this signal
 * Always connect with a blocking queued connection lambda, else the behaviour is undefined
 *
 * @var Audio::AUDIO_SAMPLE_RATE
 * @brief The next best Opus would take is 24k
 *
 * @var Audio::AUDIO_FRAME_DURATION
 * @brief In milliseconds
 *
 * @var Audio::AUDIO_FRAME_SAMPLE_COUNT
 * @brief Frame sample count
 *
 * @var Audio::AUDIO_CHANNELS
 * @brief Ideally, we'd auto-detect, but that's a sane default
 */

/**
 * @brief Returns the singleton instance.
 */
Audio& Audio::getInstance()
{
    static Audio instance;
    return instance;
}

Audio::Audio()
    : d{new Private}
    , audioThread{new QThread}
    , alInDev{nullptr}
    , inSubscriptions{0}
    , alOutDev{nullptr}
    , alOutContext{nullptr}
    , alMainSource{0}
    , alMainBuffer{0}
    , outputInitialized{false}
{
    // initialize OpenAL error stack
    alGetError();
    alcGetError(nullptr);

    audioThread->setObjectName("qTox Audio");
    QObject::connect(audioThread, &QThread::finished, audioThread, &QThread::deleteLater);

    moveToThread(audioThread);

    connect(&captureTimer, &QTimer::timeout, this, &Audio::doCapture);
    captureTimer.setInterval(AUDIO_FRAME_DURATION/2);
    captureTimer.setSingleShot(false);
    captureTimer.start();
    connect(&playMono16Timer, &QTimer::timeout, this, &Audio::playMono16SoundCleanup);
    playMono16Timer.setSingleShot(true);

    audioThread->start();
}

Audio::~Audio()
{
    audioThread->exit();
    audioThread->wait();
    cleanupInput();
    cleanupOutput();
    delete d;
}

void Audio::checkAlError() noexcept
{
    const ALenum al_err = alGetError();
    if (al_err != AL_NO_ERROR)
        qWarning("OpenAL error: %d", al_err);
}

void Audio::checkAlcError(ALCdevice *device) noexcept
{
    const ALCenum alc_err = alcGetError(device);
    if (alc_err)
        qWarning("OpenAL error: %d", alc_err);
}

/**
 * @brief Returns the current output volume (between 0 and 1)
 */
qreal Audio::outputVolume() const
{
    QMutexLocker locker(&audioLock);

    ALfloat volume = 0.0;

    if (alOutDev)
    {
        alGetListenerf(AL_GAIN, &volume);
        checkAlError();
    }

    return volume;
}

/**
 * @brief Set the master output volume.
 *
 * @param[in] volume   the master volume (between 0 and 1)
 */
void Audio::setOutputVolume(qreal volume)
{
    QMutexLocker locker(&audioLock);

    volume = std::max(0.0, std::min(volume, 1.0));

    alListenerf(AL_GAIN, static_cast<ALfloat>(volume));
    checkAlError();
}

/**
 * @brief The minimum gain value for an input device.
 *
 * @return minimum gain value in dB
 */
qreal Audio::minInputGain() const
{
    QMutexLocker locker(&audioLock);
    return d->minInputGain;
}

/**
 * @brief Set the minimum allowed gain value in dB.
 *
 * @note Default is -30dB; usually you don't need to alter this value;
 */
void Audio::setMinInputGain(qreal dB)
{
    QMutexLocker locker(&audioLock);
    d->minInputGain = dB;
}

/**
 * @brief The maximum gain value for an input device.
 *
 * @return maximum gain value in dB
 */
qreal Audio::maxInputGain() const
{
    QMutexLocker locker(&audioLock);
    return d->maxInputGain;
}

/**
 * @brief Set the maximum allowed gain value in dB.
 *
 * @note Default is 30dB; usually you don't need to alter this value.
 */
void Audio::setMaxInputGain(qreal dB)
{
    QMutexLocker locker(&audioLock);
    d->maxInputGain = dB;
}

/**
 * @brief The dB gain value.
 *
 * @return the gain value in dB
 */
qreal Audio::inputGain() const
{
    QMutexLocker locker(&audioLock);
    return d->inputGain();
}

/**
 * @brief Set the input gain dB level.
 */
void Audio::setInputGain(qreal dB)
{
    QMutexLocker locker(&audioLock);
    d->setInputGain(dB);
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
 * @brief Subscribe to capture sound from the opened input device.
 *
 * If the input device is not open, it will be opened before capturing.
 */
void Audio::subscribeInput()
{
    QMutexLocker locker(&audioLock);

    if (!autoInitInput())
    {
        qWarning("Failed to subscribe to audio input device.");
        return;
    }

    inSubscriptions++;
    qDebug() << "Subscribed to audio input device [" << inSubscriptions << "subscriptions ]";
}

/**
 * @brief Unsubscribe from capturing from an opened input device.
 *
 * If the input device has no more subscriptions, it will be closed.
 */
void Audio::unsubscribeInput()
{
    QMutexLocker locker(&audioLock);

    if (!inSubscriptions)
        return;

    inSubscriptions--;
    qDebug() << "Unsubscribed from audio input device [" << inSubscriptions << "subscriptions left ]";

    if (!inSubscriptions)
        cleanupInput();
}

/**
 * @brief Initialize audio input device, if not initialized.
 *
 * @return true, if device was initialized; false otherwise
 */
bool Audio::autoInitInput()
{
    return alInDev ? true : initInput(Settings::getInstance().getInDev());
}

/**
 * @brief Initialize audio output device, if not initialized.
 *
 * @return true, if device was initialized; false otherwise
 */
bool Audio::autoInitOutput()
{
    return alOutDev ? true : initOutput(Settings::getInstance().getOutDev());
}

bool Audio::initInput(const QString& deviceName)
{
    if (!Settings::getInstance().getAudioInDevEnabled())
        return false;

    qDebug() << "Opening audio input" << deviceName;
    assert(!alInDev);

    // TODO: Try to actually detect if our audio source is stereo
    int stereoFlag = AUDIO_CHANNELS == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    const uint32_t sampleRate = AUDIO_SAMPLE_RATE;
    const uint16_t frameDuration = AUDIO_FRAME_DURATION;
    const uint32_t chnls = AUDIO_CHANNELS;
    const ALCsizei bufSize = (frameDuration * sampleRate * 4) / 1000 * chnls;

    const ALchar* tmpDevName = deviceName.isEmpty()
                               ? nullptr
                               : deviceName.toUtf8().constData();
    alInDev = alcCaptureOpenDevice(tmpDevName, sampleRate, stereoFlag, bufSize);

    // Restart the capture if necessary
    if (!alInDev)
    {
        qWarning() << "Failed to initialize audio input device:" << deviceName;
        return false;
    }

    d->setInputGain(Settings::getInstance().getAudioInGainDecibel());

    qDebug() << "Opened audio input" << deviceName;
    alcCaptureStart(alInDev);

    return true;
}

/**
 * @brief Open an audio output device
 */
bool Audio::initOutput(const QString& deviceName)
{
    outSources.clear();

    outputInitialized = false;
    if (!Settings::getInstance().getAudioOutDevEnabled())
        return false;

    qDebug() << "Opening audio output" << deviceName;
    assert(!alOutDev);

    const ALchar* tmpDevName = deviceName.isEmpty()
                               ? nullptr
                               : deviceName.toUtf8().constData();
    alOutDev = alcOpenDevice(tmpDevName);

    if (!alOutDev)
    {
        qWarning() << "Cannot open output audio device" << deviceName;
        return false;
    }

    qDebug() << "Opened audio output" << deviceName;
    alOutContext = alcCreateContext(alOutDev, nullptr);
    checkAlcError(alOutDev);

    if (!alcMakeContextCurrent(alOutContext))
    {
        qWarning() << "Cannot create output audio context";
        return false;
    }

    alGenSources(1, &alMainSource);
    checkAlError();

    // init master volume
    alListenerf(AL_GAIN, Settings::getInstance().getOutVolume() * 0.01f);
    checkAlError();

    Core* core = Core::getInstance();
    if (core)
    {
        // reset each call's audio source
        core->getAv()->invalidateCallSources();
    }

    outputInitialized = true;
    return true;
}

/**
 * @brief Play a 44100Hz mono 16bit PCM sound from a file
 */
void Audio::playMono16Sound(const QString& path)
{
    QFile sndFile(path);
    sndFile.open(QIODevice::ReadOnly);
    playMono16Sound(QByteArray(sndFile.readAll()));
}

/**
 * @brief Play a 44100Hz mono 16bit PCM sound
 */
void Audio::playMono16Sound(const QByteArray& data)
{
    QMutexLocker locker(&audioLock);

    if (!autoInitOutput())
        return;

    if (!alMainBuffer)
        alGenBuffers(1, &alMainBuffer);

    ALint state;
    alGetSourcei(alMainSource, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING)
    {
        alSourceStop(alMainSource);
        alSourcei(alMainSource, AL_BUFFER, AL_NONE);
    }

    alBufferData(alMainBuffer, AL_FORMAT_MONO16, data.constData(), data.size(), 44100);
    alSourcei(alMainSource, AL_BUFFER, static_cast<ALint>(alMainBuffer));
    alSourcePlay(alMainSource);

    int durationMs = data.size() * 1000 / 2 / 44100;
    playMono16Timer.start(durationMs + 50);
}

void Audio::playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate)
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
 * @brief Close active audio input device.
 */
void Audio::cleanupInput()
{
    if (!alInDev)
        return;

    qDebug() << "Closing audio input";
    alcCaptureStop(alInDev);
    if (alcCaptureCloseDevice(alInDev) == ALC_TRUE)
        alInDev = nullptr;
    else
        qWarning() << "Failed to close input";
}

/**
 * @brief Close active audio output device
 */
void Audio::cleanupOutput()
{
    outputInitialized = false;

    if (alOutDev)
    {
        alSourcei(alMainSource, AL_LOOPING, AL_FALSE);
        alSourceStop(alMainSource);
        alDeleteSources(1, &alMainSource);

        if (alMainBuffer)
        {
            alDeleteBuffers(1, &alMainBuffer);
            alMainBuffer = 0;
        }

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
 * @brief Called after a mono16 sound stopped playing
 */
void Audio::playMono16SoundCleanup()
{
    QMutexLocker locker(&audioLock);

    ALint state;
    alGetSourcei(alMainSource, AL_SOURCE_STATE, &state);
    if (state == AL_STOPPED)
    {
        alSourcei(alMainSource, AL_BUFFER, AL_NONE);
        alDeleteBuffers(1, &alMainBuffer);
        alMainBuffer = 0;
    }
}

/**
 * @brief Called on the captureTimer events to capture audio
 */
void Audio::doCapture()
{
    QMutexLocker lock(&audioLock);

    if (!alInDev || !inSubscriptions)
        return;

    ALint curSamples = 0;
    alcGetIntegerv(alInDev, ALC_CAPTURE_SAMPLES, sizeof(curSamples), &curSamples);
    if (curSamples < AUDIO_FRAME_SAMPLE_COUNT)
        return;

    int16_t buf[AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS];
    alcCaptureSamples(alInDev, buf, AUDIO_FRAME_SAMPLE_COUNT);

    for (quint32 i = 0; i < AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS; ++i)
    {
        // gain amplification with clipping to 16-bit boundaries
        int ampPCM = qBound<int>(std::numeric_limits<int16_t>::min(),
                                 qRound(buf[i] * d->inputGainFactor()),
                                 std::numeric_limits<int16_t>::max());

        buf[i] = static_cast<int16_t>(ampPCM);
    }

    emit frameAvailable(buf, AUDIO_FRAME_SAMPLE_COUNT, AUDIO_CHANNELS, AUDIO_SAMPLE_RATE);
}

/**
 * @brief Returns true if the output device is open
 */
bool Audio::isOutputReady() const
{
    QMutexLocker locker(&audioLock);
    return alOutDev && outputInitialized;
}

QStringList Audio::outDeviceNames()
{
    QStringList list;
    const ALchar* pDeviceList = Private::outDeviceNames();

    if (pDeviceList)
    {
        while (*pDeviceList)
        {
            int len = static_cast<int>(strlen(pDeviceList));
            list << QString::fromUtf8(pDeviceList, len);
            pDeviceList += len+1;
        }
    }

    return list;
}

QStringList Audio::inDeviceNames()
{
    QStringList list;
    const ALchar* pDeviceList = Private::inDeviceNames();

    if (pDeviceList)
    {
        while (*pDeviceList)
        {
            int len = static_cast<int>(strlen(pDeviceList));
            list << QString::fromUtf8(pDeviceList, len);
            pDeviceList += len+1;
        }
    }

    return list;
}

void Audio::subscribeOutput(ALuint& sid)
{
    QMutexLocker locker(&audioLock);

    if (!autoInitOutput())
    {
        qWarning("Failed to subscribe to audio output device.");
        return;
    }

    if (!alcMakeContextCurrent(alOutContext))
    {
        qWarning("Failed to activate output context.");
        return;
    }

    alGenSources(1, &sid);
    assert(sid);
    outSources << sid;

    qDebug() << "Audio source" << sid << "created. Sources active:"
             << outSources.size();
}

void Audio::unsubscribeOutput(ALuint &sid)
{
    QMutexLocker locker(&audioLock);

    outSources.removeAll(sid);

    if (sid)
    {
        if (alIsSource(sid))
        {
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
