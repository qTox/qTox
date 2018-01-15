/*
    Copyright © 2014-2017 by The qTox Project Contributors

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

#include "openal.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/persistence/settings.h"

#include <QDebug>
#include <QFile>
#include <QMutexLocker>
#include <QPointer>
#include <QThread>
#include <QWaitCondition>
#include <QtMath>

#include <cassert>

/**
 * @class OpenAL
 * @brief Provides the OpenAL audio backend
 *
 * @var BUFFER_COUNT
 * @brief Number of buffers to use per audio source
 *
 * @var AUDIO_CHANNELS
 * @brief Ideally, we'd auto-detect, but that's a sane default
 */

static const unsigned int BUFFER_COUNT = 16;
static const uint32_t AUDIO_CHANNELS = 2;

OpenAL::OpenAL()
    : audioThread{new QThread}
{
    // initialize OpenAL error stack
    alGetError();
    alcGetError(nullptr);

    audioThread->setObjectName("qTox Audio");
    QObject::connect(audioThread, &QThread::finished, audioThread, &QThread::deleteLater);

    moveToThread(audioThread);

    voiceTimer.setSingleShot(true);
    connect(this, &Audio::startActive, &voiceTimer, static_cast<void (QTimer::*)(int)>(&QTimer::start));
    connect(&voiceTimer, &QTimer::timeout, this, &Audio::stopActive);

    connect(&captureTimer, &QTimer::timeout, this, &OpenAL::doAudio);
    captureTimer.setInterval(AUDIO_FRAME_DURATION / 2);
    captureTimer.setSingleShot(false);
    captureTimer.start();
    connect(&playMono16Timer, &QTimer::timeout, this, &OpenAL::playMono16SoundCleanup);
    playMono16Timer.setSingleShot(true);

    audioThread->start();
}

OpenAL::~OpenAL()
{
    audioThread->exit();
    audioThread->wait();
    cleanupInput();
    cleanupOutput();
}

void OpenAL::checkAlError() noexcept
{
    const ALenum al_err = alGetError();
    if (al_err != AL_NO_ERROR)
        qWarning("OpenAL error: %d", al_err);
}

void OpenAL::checkAlcError(ALCdevice* device) noexcept
{
    const ALCenum alc_err = alcGetError(device);
    if (alc_err)
        qWarning("OpenAL error: %d", alc_err);
}

/**
 * @brief Returns the current output volume (between 0 and 1)
 */
qreal OpenAL::outputVolume() const
{
    QMutexLocker locker(&audioLock);

    ALfloat volume = 0.0;

    if (alOutDev) {
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
void OpenAL::setOutputVolume(qreal volume)
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
qreal OpenAL::minInputGain() const
{
    QMutexLocker locker(&audioLock);
    return minInGain;
}

/**
 * @brief Set the minimum allowed gain value in dB.
 *
 * @note Default is -30dB; usually you don't need to alter this value;
 */
void OpenAL::setMinInputGain(qreal dB)
{
    QMutexLocker locker(&audioLock);
    minInGain = dB;
}

/**
 * @brief The maximum gain value for an input device.
 *
 * @return maximum gain value in dB
 */
qreal OpenAL::maxInputGain() const
{
    QMutexLocker locker(&audioLock);
    return maxInGain;
}

/**
 * @brief Set the maximum allowed gain value in dB.
 *
 * @note Default is 30dB; usually you don't need to alter this value.
 */
void OpenAL::setMaxInputGain(qreal dB)
{
    QMutexLocker locker(&audioLock);
    maxInGain = dB;
}

/**
 * @brief The minimum threshold value for an input device.
 *
 * @return minimum threshold percentage
 */
qreal OpenAL::minInputThreshold() const
{
    QMutexLocker locker(&audioLock);
    return minInThreshold;
}

/**
 * @brief Set the minimum allowed threshold percentage
 *
 * @note Default is 0%; usually you don't need to alter this value;
 */
void OpenAL::setMinInputThreshold(qreal percent)
{
    QMutexLocker locker(&audioLock);
    minInThreshold = percent;
}

/**
 * @brief The maximum threshold value for an input device.
 *
 * @return maximum threshold percentage
 */
qreal OpenAL::maxInputThreshold() const
{
    QMutexLocker locker(&audioLock);
    return maxInThreshold;
}

/**
 * @brief Set the maximum allowed threshold percentage
 *
 * @note Default is 40%; usually you don't need to alter this value.
 */
void OpenAL::setMaxInputThreshold(qreal percent)
{
    QMutexLocker locker(&audioLock);
    maxInThreshold = percent;
}

void OpenAL::reinitInput(const QString& inDevDesc)
{
    QMutexLocker locker(&audioLock);
    cleanupInput();
    initInput(inDevDesc);
}

bool OpenAL::reinitOutput(const QString& outDevDesc)
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
void OpenAL::subscribeInput()
{
    QMutexLocker locker(&audioLock);

    if (!autoInitInput()) {
        qWarning("Failed to subscribe to audio input device.");
        return;
    }

    ++inSubscriptions;
    qDebug() << "Subscribed to audio input device [" << inSubscriptions << "subscriptions ]";
}

/**
 * @brief Unsubscribe from capturing from an opened input device.
 *
 * If the input device has no more subscriptions, it will be closed.
 */
void OpenAL::unsubscribeInput()
{
    QMutexLocker locker(&audioLock);

    if (!inSubscriptions)
        return;

    inSubscriptions--;
    qDebug() << "Unsubscribed from audio input device [" << inSubscriptions
             << "subscriptions left ]";

    if (!inSubscriptions)
        cleanupInput();
}

/**
 * @brief Initialize audio input device, if not initialized.
 *
 * @return true, if device was initialized; false otherwise
 */
bool OpenAL::autoInitInput()
{
    return alInDev ? true : initInput(Settings::getInstance().getInDev());
}

/**
 * @brief Initialize audio output device, if not initialized.
 *
 * @return true, if device was initialized; false otherwise
 */
bool OpenAL::autoInitOutput()
{
    return alOutDev ? true : initOutput(Settings::getInstance().getOutDev());
}

bool OpenAL::initInput(const QString& deviceName)
{
    return initInput(deviceName, AUDIO_CHANNELS);
}

bool OpenAL::initInput(const QString& deviceName, uint32_t channels)
{
    if (!Settings::getInstance().getAudioInDevEnabled()) {
        return false;
    }

    qDebug() << "Opening audio input" << deviceName;
    assert(!alInDev);

    // TODO: Try to actually detect if our audio source is stereo
    int stereoFlag = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    const uint32_t sampleRate = AUDIO_SAMPLE_RATE;
    const uint16_t frameDuration = AUDIO_FRAME_DURATION;
    const ALCsizei bufSize = (frameDuration * sampleRate * 4) / 1000 * channels;

    const QByteArray qDevName = deviceName.toUtf8();
    const ALchar* tmpDevName = qDevName.isEmpty() ? nullptr : qDevName.constData();
    alInDev = alcCaptureOpenDevice(tmpDevName, sampleRate, stereoFlag, bufSize);

    // Restart the capture if necessary
    if (!alInDev) {
        qWarning() << "Failed to initialize audio input device:" << deviceName;
        return false;
    }

    setInputGain(Settings::getInstance().getAudioInGainDecibel());
    setInputThreshold(Settings::getInstance().getAudioThreshold());

    qDebug() << "Opened audio input" << deviceName;
    alcCaptureStart(alInDev);

    return true;
}

/**
 * @brief Open an audio output device
 */
bool OpenAL::initOutput(const QString& deviceName)
{
    peerSources.clear();

    outputInitialized = false;
    if (!Settings::getInstance().getAudioOutDevEnabled())
        return false;

    qDebug() << "Opening audio output" << deviceName;
    assert(!alOutDev);

    const QByteArray qDevName = deviceName.toUtf8();
    const ALchar* tmpDevName = qDevName.isEmpty() ? nullptr : qDevName.constData();
    alOutDev = alcOpenDevice(tmpDevName);

    if (!alOutDev) {
        qWarning() << "Cannot open output audio device" << deviceName;
        return false;
    }

    qDebug() << "Opened audio output" << deviceName;
    alOutContext = alcCreateContext(alOutDev, nullptr);
    checkAlcError(alOutDev);

    if (!alcMakeContextCurrent(alOutContext)) {
        qWarning() << "Cannot create output audio context";
        return false;
    }

    alGenSources(1, &alMainSource);
    checkAlError();

    // init master volume
    alListenerf(AL_GAIN, Settings::getInstance().getOutVolume() * 0.01f);
    checkAlError();

    Core* core = Core::getInstance();
    if (core) {
        // reset each call's audio source
        core->getAv()->invalidateCallSources();
    }

    outputInitialized = true;
    return true;
}

/**
 * @brief Play a 44100Hz mono 16bit PCM sound from a file
 *
 * @param[in] path the path to the sound file
 */
void OpenAL::playMono16Sound(const QString& path)
{
    QFile sndFile(path);
    sndFile.open(QIODevice::ReadOnly);
    playMono16Sound(QByteArray(sndFile.readAll()));
}

/**
 * @brief Play a 44100Hz mono 16bit PCM sound
 */
void OpenAL::playMono16Sound(const QByteArray& data)
{
    QMutexLocker locker(&audioLock);

    if (!autoInitOutput())
        return;

    if (!alMainBuffer)
        alGenBuffers(1, &alMainBuffer);

    ALint state;
    alGetSourcei(alMainSource, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) {
        alSourceStop(alMainSource);
        alSourcei(alMainSource, AL_BUFFER, AL_NONE);
    }

    alBufferData(alMainBuffer, AL_FORMAT_MONO16, data.constData(), data.size(), 44100);
    alSourcei(alMainSource, AL_BUFFER, static_cast<ALint>(alMainBuffer));
    alSourcePlay(alMainSource);

    int durationMs = data.size() * 1000 / 2 / 44100;
    playMono16Timer.start(durationMs + 50);
}

void OpenAL::playAudioBuffer(uint sourceId, const int16_t* data, int samples, unsigned channels,
                             int sampleRate)
{
    assert(channels == 1 || channels == 2);
    QMutexLocker locker(&audioLock);

    if (!(alOutDev && outputInitialized))
        return;

    ALuint bufids[BUFFER_COUNT];
    ALint processed = 0, queued = 0;
    alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(sourceId, AL_BUFFERS_QUEUED, &queued);
    alSourcei(sourceId, AL_LOOPING, AL_FALSE);

    if (processed == 0) {
        if (static_cast<ALuint>(queued) >= BUFFER_COUNT) {
            // reached limit, drop audio
            return;
        }
        // create new buffer if none got free and we're below the limit
        alGenBuffers(1, bufids);
    } else {
        // unqueue all processed buffers
        alSourceUnqueueBuffers(sourceId, processed, bufids);
        // delete all but the first buffer, reuse first for new data
        alDeleteBuffers(processed - 1, bufids + 1);
    }

    alBufferData(bufids[0], (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data,
                 samples * 2 * channels, sampleRate);
    alSourceQueueBuffers(sourceId, 1, bufids);

    ALint state;
    alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        alSourcePlay(sourceId);
    }
}

/**
 * @brief Close active audio input device.
 */
void OpenAL::cleanupInput()
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
void OpenAL::cleanupOutput()
{
    outputInitialized = false;

    if (alOutDev) {
        alSourcei(alMainSource, AL_LOOPING, AL_FALSE);
        alSourceStop(alMainSource);
        alDeleteSources(1, &alMainSource);

        if (alMainBuffer) {
            alDeleteBuffers(1, &alMainBuffer);
            alMainBuffer = 0;
        }

        if (!alcMakeContextCurrent(nullptr)) {
            qWarning("Failed to clear audio context.");
        }

        alcDestroyContext(alOutContext);
        alOutContext = nullptr;

        qDebug() << "Closing audio output";
        if (alcCloseDevice(alOutDev)) {
            alOutDev = nullptr;
        } else {
            qWarning("Failed to close output.");
        }
    }
}

/**
 * @brief Called after a mono16 sound stopped playing
 */
void OpenAL::playMono16SoundCleanup()
{
    QMutexLocker locker(&audioLock);

    ALint state;
    alGetSourcei(alMainSource, AL_SOURCE_STATE, &state);
    if (state == AL_STOPPED) {
        alSourcei(alMainSource, AL_BUFFER, AL_NONE);
        alDeleteBuffers(1, &alMainBuffer);
        alMainBuffer = 0;
    } else {
        // the audio didn't finish, try again later
        playMono16Timer.start(10);
    }
}

/**
 * @brief Called by doCapture to calculate volume of the audio buffer
 *
 * @param[in] buf   the current audio buffer
 *
 * @return volume in percent of max volume
 */
float OpenAL::getVolume(int16_t *buf)
{
    quint32 samples = AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS;
    float sum = 0.0;
    for (quint32 i = 0; i < samples; i++) {
        float sample = (float)buf[i] / (float)std::numeric_limits<int16_t>::max();
        if (sample > 0) {
            sum += sample;
        } else {
            sum -= sample;
        }
    }
    return sum/samples;
}

/**
 * @brief Called by voiceTimer's timeout to disable audio broadcasting
 */
void OpenAL::stopActive()
{
    isActive = false;
}

/**
 * @brief Called on the captureTimer events to capture audio
 */
void OpenAL::doAudio()
{
    QMutexLocker lock(&audioLock);

    if (!alInDev || !inSubscriptions)
        return;

    ALint curSamples = 0;
    alcGetIntegerv(alInDev, ALC_CAPTURE_SAMPLES, sizeof(curSamples), &curSamples);
    if (static_cast<ALuint>(curSamples) < AUDIO_FRAME_SAMPLE_COUNT)
        return;

    int16_t buf[AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS];
    alcCaptureSamples(alInDev, buf, AUDIO_FRAME_SAMPLE_COUNT);

    float volume = getVolume(buf);
    if (volume >= inputThreshold)
    {
        isActive = true;
        emit startActive(voiceHold);
    }

    emit Audio::volumeAvailable(volume);
    if (!isActive)
    {
        return;
    }

    for (quint32 i = 0; i < AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS; ++i) {
        // gain amplification with clipping to 16-bit boundaries
        int ampPCM =
            qBound<int>(std::numeric_limits<int16_t>::min(), qRound(buf[i] * inputGainFactor()),
                        std::numeric_limits<int16_t>::max());

        buf[i] = static_cast<int16_t>(ampPCM);
    }

    emit Audio::frameAvailable(buf, AUDIO_FRAME_SAMPLE_COUNT, AUDIO_CHANNELS, AUDIO_SAMPLE_RATE);
}

/**
 * @brief Returns true if the output device is open
 */
bool OpenAL::isOutputReady() const
{
    QMutexLocker locker(&audioLock);
    return alOutDev && outputInitialized;
}

QStringList OpenAL::outDeviceNames()
{
    QStringList list;
    const ALchar* pDeviceList = (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
                                    ? alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER)
                                    : alcGetString(NULL, ALC_DEVICE_SPECIFIER);

    if (pDeviceList) {
        while (*pDeviceList) {
            int len = static_cast<int>(strlen(pDeviceList));
            list << QString::fromUtf8(pDeviceList, len);
            pDeviceList += len + 1;
        }
    }

    return list;
}

QStringList OpenAL::inDeviceNames()
{
    QStringList list;
    const ALchar* pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

    if (pDeviceList) {
        while (*pDeviceList) {
            int len = static_cast<int>(strlen(pDeviceList));
            list << QString::fromUtf8(pDeviceList, len);
            pDeviceList += len + 1;
        }
    }

    return list;
}

void OpenAL::subscribeOutput(uint& sid)
{
    QMutexLocker locker(&audioLock);

    if (!autoInitOutput()) {
        qWarning("Failed to subscribe to audio output device.");
        return;
    }

    alGenSources(1, &sid);
    assert(sid);
    peerSources << sid;

    qDebug() << "Audio source" << sid << "created. Sources active:" << peerSources.size();
}

void OpenAL::unsubscribeOutput(uint& sid)
{
    QMutexLocker locker(&audioLock);

    peerSources.removeAll(sid);

    if (sid) {
        if (alIsSource(sid)) {
            // stop playing, marks all buffers as processed
            alSourceStop(sid);
            // unqueue all buffers from the source
            ALint processed = 0;
            alGetSourcei(sid, AL_BUFFERS_PROCESSED, &processed);
            ALuint* bufids = new ALuint[processed];
            alSourceUnqueueBuffers(sid, processed, bufids);
            // delete all buffers
            alDeleteBuffers(processed, bufids);
            delete[] bufids;
            alDeleteSources(1, &sid);
            qDebug() << "Audio source" << sid << "deleted. Sources active:" << peerSources.size();
        } else {
            qWarning() << "Trying to delete invalid audio source" << sid;
        }

        sid = 0;
    }

    if (peerSources.isEmpty())
        cleanupOutput();
}

void OpenAL::startLoop()
{
    QMutexLocker locker(&audioLock);
    alSourcei(alMainSource, AL_LOOPING, AL_TRUE);
}

void OpenAL::stopLoop()
{
    QMutexLocker locker(&audioLock);
    alSourcei(alMainSource, AL_LOOPING, AL_FALSE);
    alSourceStop(alMainSource);

    ALint state;
    alGetSourcei(alMainSource, AL_SOURCE_STATE, &state);
    if (state == AL_STOPPED) {
        alSourcei(alMainSource, AL_BUFFER, AL_NONE);
        alDeleteBuffers(1, &alMainBuffer);
        alMainBuffer = 0;
    }
}

qreal OpenAL::inputGain() const
{
    return gain;
}

qreal OpenAL::getInputThreshold() const
{
    return inputThreshold;
}

qreal OpenAL::inputGainFactor() const
{
    return gainFactor;
}

void OpenAL::setInputGain(qreal dB)
{
    gain = qBound(minInGain, dB, maxInGain);
    gainFactor = qPow(10.0, (gain / 20.0));
}

void OpenAL::setInputThreshold(qreal percent)
{
    inputThreshold = percent;
}

