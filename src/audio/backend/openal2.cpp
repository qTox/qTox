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

#include "openal2.h"
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

extern "C" {
#include <filter_audio.h>
}

/**
 * @class OpenAL
 * @brief Provides the OpenAL audio backend
 *
 * @var BUFFER_COUNT
 * @brief Number of buffers to use per audio source
 */

static const unsigned int BUFFER_COUNT = 16;
static const unsigned int PROXY_BUFFER_COUNT = 4;

OpenAL2::OpenAL2() :
      audioThread{new QThread}
    , alInDev{nullptr}
    , inSubscriptions{0}
    , alOutDev{nullptr}
    , alOutContext{nullptr}
    , alProxyDev{nullptr}
    , alProxyContext{nullptr}
    , alMainSource{0}
    , alMainBuffer{0}
    , alProxySource{0}
    , alProxyBuffer{0}
    , outputInitialized{false}
{
    // initialize OpenAL error stack
    alGetError();
    alcGetError(nullptr);

    audioThread->setObjectName("qTox Audio");
    QObject::connect(audioThread, &QThread::finished, audioThread, &QThread::deleteLater);

    moveToThread(audioThread);

    connect(&captureTimer, &QTimer::timeout, this, &OpenAL2::doAudio);
    captureTimer.setInterval(AUDIO_FRAME_DURATION / 2);
    captureTimer.setSingleShot(false);
    captureTimer.start();
    connect(&playMono16Timer, &QTimer::timeout, this, &OpenAL2::playMono16SoundCleanup);
    playMono16Timer.setSingleShot(true);

    audioThread->start();
}

OpenAL2::~OpenAL2()
{
    audioThread->exit();
    audioThread->wait();
    cleanupInput();
    cleanupOutput();
}

void OpenAL2::checkAlError() noexcept
{
    const ALenum al_err = alGetError();
    if (al_err != AL_NO_ERROR)
        qWarning("OpenAL error: %d", al_err);
}

void OpenAL2::checkAlcError(ALCdevice* device) noexcept
{
    const ALCenum alc_err = alcGetError(device);
    if (alc_err)
        qWarning("OpenAL error: %d", alc_err);
}

/**
 * @brief Returns the current output volume (between 0 and 1)
 */
qreal OpenAL2::outputVolume() const
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
void OpenAL2::setOutputVolume(qreal volume)
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
qreal OpenAL2::minInputGain() const
{
    QMutexLocker locker(&audioLock);
    return minInGain;
}

/**
 * @brief Set the minimum allowed gain value in dB.
 *
 * @note Default is -30dB; usually you don't need to alter this value;
 */
void OpenAL2::setMinInputGain(qreal dB)
{
    QMutexLocker locker(&audioLock);
    minInGain = dB;
}

/**
 * @brief The maximum gain value for an input device.
 *
 * @return maximum gain value in dB
 */
qreal OpenAL2::maxInputGain() const
{
    QMutexLocker locker(&audioLock);
    return maxInGain;
}

/**
 * @brief Set the maximum allowed gain value in dB.
 *
 * @note Default is 30dB; usually you don't need to alter this value.
 */
void OpenAL2::setMaxInputGain(qreal dB)
{
    QMutexLocker locker(&audioLock);
    maxInGain = dB;
}

void OpenAL2::reinitInput(const QString& inDevDesc)
{
    QMutexLocker locker(&audioLock);
    cleanupInput();
    initInput(inDevDesc);
}

bool OpenAL2::reinitOutput(const QString& outDevDesc)
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
void OpenAL2::subscribeInput()
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
void OpenAL2::unsubscribeInput()
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
bool OpenAL2::autoInitInput()
{
    return alInDev ? true : initInput(Settings::getInstance().getInDev());
}

/**
 * @brief Initialize audio output device, if not initialized.
 *
 * @return true, if device was initialized; false otherwise
 */
bool OpenAL2::autoInitOutput()
{
    return alOutDev ? true : initOutput(Settings::getInstance().getOutDev());
}

bool OpenAL2::initInput(const QString& deviceName)
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

    const QByteArray qDevName = deviceName.toUtf8();
    const ALchar* tmpDevName = qDevName.isEmpty() ? nullptr : qDevName.constData();
    alInDev = alcCaptureOpenDevice(tmpDevName, sampleRate, stereoFlag, bufSize);

    // Restart the capture if necessary
    if (!alInDev) {
        qWarning() << "Failed to initialize audio input device:" << deviceName;
        return false;
    }

    setInputGain(Settings::getInstance().getAudioInGainDecibel());

    qDebug() << "Opened audio input" << deviceName;
    alcCaptureStart(alInDev);

    return true;
}

/**
 * @brief Loads the OpenAL extension methods needed for echo cancellation
 * @param dev OpenAL device used for the output
 * @return True when functions successfully loaded, false otherwise
 */
bool OpenAL2::loadOpenALExtensions(ALCdevice* dev)
{
    // load OpenAL extension functions
    alcLoopbackOpenDeviceSOFT = reinterpret_cast<LPALCLOOPBACKOPENDEVICESOFT>
            (alcGetProcAddress(dev, "alcLoopbackOpenDeviceSOFT"));

    checkAlcError(dev);
    if(!alcLoopbackOpenDeviceSOFT) {
        qDebug() << "Failed to load alcLoopbackOpenDeviceSOFT function!";
        return false;
    }
    alcIsRenderFormatSupportedSOFT = reinterpret_cast<LPALCISRENDERFORMATSUPPORTEDSOFT>
            (alcGetProcAddress(dev, "alcIsRenderFormatSupportedSOFT"));

    checkAlcError(dev);
    if(!alcIsRenderFormatSupportedSOFT) {
        qDebug() << "Failed to load alcIsRenderFormatSupportedSOFT function!";
        return false;
    }
    alGetSourcedvSOFT = reinterpret_cast<LPALGETSOURCEDVSOFT>
            (alcGetProcAddress(dev, "alGetSourcedvSOFT"));

    checkAlcError(dev);
    if(!alGetSourcedvSOFT) {
        qDebug() << "Failed to load alGetSourcedvSOFT function!";
        return false;
    }

    return true;
}

/**
 * @brief Initializes the output with echo cancelling enabled
 * @return true on success, false otherwise
 * Creates a loopback device and a proxy source on the main output device.
 * If this function returns true only the proxy source should be used for
 * audio output.
 */
bool OpenAL2::initOutputEchoCancel()
{
    // check for the needed extensions for echo cancelation
    if (alcIsExtensionPresent(alOutDev, "ALC_SOFT_loopback") != AL_TRUE) {
        qDebug() << "Device doesn't support loopback";
        return false;
    }
    if (alIsExtensionPresent("AL_SOFT_source_latency") != AL_TRUE) {
        qDebug() << "Device doesn't support source latency";
        return false;
    }

    if(!loadOpenALExtensions(alOutDev)) {
        qDebug() << "Couldn't load needed OpenAL extensions";
        return false;
    }

    // source for proxy output
    alGenSources(1, &alProxySource);
    checkAlError();

    // configuration for the loopback device
    ALCint attrs[] = { ALC_FORMAT_CHANNELS_SOFT, AUDIO_CHANNELS == 1 ? ALC_MONO_SOFT : ALC_STEREO_SOFT,
                     ALC_FORMAT_TYPE_SOFT, ALC_SHORT_SOFT,
                     ALC_FREQUENCY, Audio::AUDIO_SAMPLE_RATE,
                     0 }; // End of List

    alProxyDev = alcLoopbackOpenDeviceSOFT(NULL);
    checkAlcError(alProxyDev);
    if(!alProxyDev) {
        qDebug() << "Couldn't create proxy device";
        alDeleteSources(1, &alProxySource); // cleanup source
        return false;
    }

    if(!alcIsRenderFormatSupportedSOFT(alProxyDev, attrs[5], attrs[1], attrs[3])) {
        qDebug() << "Unsupported format for loopback";
        alcCloseDevice(alProxyDev);         // cleanup loopback dev
        alDeleteSources(1, &alProxySource); // cleanup source
        return false;
    }

    alProxyContext = alcCreateContext(alProxyDev, attrs);
    checkAlcError(alProxyDev);
    if(!alProxyContext) {
        qDebug() << "Couldn't create proxy context";
        alcCloseDevice(alProxyDev);         // cleanup loopback dev
        alDeleteSources(1, &alProxySource); // cleanup source
        return false;
    }

    if (!alcMakeContextCurrent(alProxyContext)) {
        qDebug() << "Cannot activate proxy context";
        alcDestroyContext(alProxyContext);
        alcCloseDevice(alProxyDev);         // cleanup loopback dev
        alDeleteSources(1, &alProxySource); // cleanup source
        return false;
    }

    qDebug() << "Echo cancelation enabled";
    return true;
}

/**
 * @brief Open an audio output device
 */
bool OpenAL2::initOutput(const QString& deviceName)
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

    // try to init echo cancellation
    echoCancelSupported = initOutputEchoCancel();

    if(!echoCancelSupported) {
        // fallback to normal, no proxy device needed
        qDebug() << "Echo cancellation disabled";
        alProxyDev = alOutDev;
        alProxyContext = alOutContext;
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
void OpenAL2::playMono16Sound(const QString& path)
{
    QFile sndFile(path);
    sndFile.open(QIODevice::ReadOnly);
    playMono16Sound(QByteArray(sndFile.readAll()));
}

/**
 * @brief Play a 44100Hz mono 16bit PCM sound
 */
void OpenAL2::playMono16Sound(const QByteArray& data)
{
    QMutexLocker locker(&audioLock);

    if (!autoInitOutput())
        return;

    alcMakeContextCurrent(alProxyContext);
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

void OpenAL2::playAudioBuffer(uint sourceId, const int16_t* data, int samples, unsigned channels,
                            int sampleRate)
{
    assert(channels == 1 || channels == 2);
    QMutexLocker locker(&audioLock);

    if (!(alOutDev && outputInitialized))
        return;

    alcMakeContextCurrent(alProxyContext);

    ALuint bufids[BUFFER_COUNT];
    ALint processed = 0, queued = 0;
    alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(sourceId, AL_BUFFERS_QUEUED, &queued);
    alSourcei(sourceId, AL_LOOPING, AL_FALSE);

    if (processed == 0) {
        if (queued >= BUFFER_COUNT) {
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
void OpenAL2::cleanupInput()
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
void OpenAL2::cleanupOutput()
{
    outputInitialized = false;

    if (alProxyDev) {
        alSourcei(alMainSource, AL_LOOPING, AL_FALSE);
        alSourceStop(alMainSource);
        alDeleteSources(1, &alMainSource);

        if (alMainBuffer) {
            alDeleteBuffers(1, &alMainBuffer);
            alMainBuffer = 0;
        }

        if (!alcMakeContextCurrent(nullptr))
            qWarning("Failed to clear audio context.");

        alcDestroyContext(alOutContext);
        alProxyContext = nullptr;

        qDebug() << "Closing audio output";
        if (alcCloseDevice(alProxyDev))
            alProxyDev = nullptr;
        else
            qWarning("Failed to close output.");
    }

    if (echoCancelSupported) {
        alcMakeContextCurrent(alOutContext);
        alSourceStop(alProxySource);
        //TODO: delete buffers
        alcDestroyContext(alOutContext);
        alOutContext = nullptr;
        alcCloseDevice(alOutDev);
        alOutDev = nullptr;
    } else {
        alOutContext = nullptr;
        alOutDev = nullptr;
    }
}

/**
 * @brief Called after a mono16 sound stopped playing
 */
void OpenAL2::playMono16SoundCleanup()
{
    QMutexLocker locker(&audioLock);

    ALint state;
    alGetSourcei(alMainSource, AL_SOURCE_STATE, &state);
    if (state == AL_STOPPED) {
        alSourcei(alMainSource, AL_BUFFER, AL_NONE);
        alDeleteBuffers(1, &alMainBuffer);
        alMainBuffer = 0;
    }
    else
    {
        // the audio didn't finish, try again later
        playMono16Timer.start(10);
    }
}

/**
 * @brief Handle audio output
 */
void OpenAL2::doOutput()
{
    alcMakeContextCurrent(alOutContext);
    ALuint bufids[PROXY_BUFFER_COUNT];
    ALint processed = 0, queued = 0;
    alGetSourcei(alProxySource, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(alProxySource, AL_BUFFERS_QUEUED, &queued);

    //qDebug() << "Speedtest processed: " << processed << " queued: " << queued;

    if (processed > 0) {
        // unqueue all processed buffers
        alSourceUnqueueBuffers(alProxySource, 1, bufids);
    } else if (queued < PROXY_BUFFER_COUNT) {
        // create new buffer until the maximum is reached
        alGenBuffers(1, bufids);
    } else {
        return;
    }

    ALdouble latency[2] = {0};
    if(alGetSourcedvSOFT) {
        alGetSourcedvSOFT(alProxySource, AL_SEC_OFFSET_LATENCY_SOFT, latency);
        checkAlError();
    }
    //qDebug() << "Playback latency: " << latency[1] << "offset: " << latency[0];

    ALshort outBuf[AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS] = {0};
    alcMakeContextCurrent(alProxyContext);
    LPALCRENDERSAMPLESSOFT alcRenderSamplesSOFT =
            reinterpret_cast<LPALCRENDERSAMPLESSOFT> (alcGetProcAddress(alOutDev, "alcRenderSamplesSOFT"));
    alcRenderSamplesSOFT(alProxyDev, outBuf, AUDIO_FRAME_SAMPLE_COUNT);

    alcMakeContextCurrent(alOutContext);
    alBufferData(bufids[0], (AUDIO_CHANNELS == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, outBuf,
                 AUDIO_FRAME_SAMPLE_COUNT * 2 * AUDIO_CHANNELS, AUDIO_SAMPLE_RATE);
    alSourceQueueBuffers(alProxySource, 1, bufids);

    // initialize echo canceler if supported
    if(!filterer) {
        filterer = new_filter_audio(AUDIO_SAMPLE_RATE);
        int16_t filterLatency = latency[1]*1000*2 + AUDIO_FRAME_DURATION;
        qDebug() << "Setting filter delay to: " << filterLatency << "ms";
        set_echo_delay_ms(filterer, filterLatency);
        enable_disable_filters(filterer, 1, 1, 1, 0);
    }

    // do echo cancel
    int retVal = pass_audio_output(filterer, outBuf, AUDIO_FRAME_SAMPLE_COUNT);

    ALint state;
    alGetSourcei(alProxySource, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        qDebug() << "Proxy source underflow detected";
        alSourcePlay(alProxySource);
    }
}

/**
 * @brief handles recording of audio frames
 */
void OpenAL2::doInput()
{
    ALint curSamples = 0;
    alcGetIntegerv(alInDev, ALC_CAPTURE_SAMPLES, sizeof(curSamples), &curSamples);
    if (curSamples < AUDIO_FRAME_SAMPLE_COUNT) {
        return;
    }

    int16_t buf[AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS];
    alcCaptureSamples(alInDev, buf, AUDIO_FRAME_SAMPLE_COUNT);

    int retVal = 0;
    if(echoCancelSupported && filterer) {
        retVal = filter_audio(filterer, buf, AUDIO_FRAME_SAMPLE_COUNT);
    }

    // gain amplification with clipping to 16-bit boundaries
    for (quint32 i = 0; i < AUDIO_FRAME_SAMPLE_COUNT * AUDIO_CHANNELS; ++i) {
        int ampPCM =
            qBound<int>(std::numeric_limits<int16_t>::min(), qRound(buf[i] * inputGainFactor()),
                        std::numeric_limits<int16_t>::max());

        buf[i] = static_cast<int16_t>(ampPCM);
    }

    emit Audio::frameAvailable(buf, AUDIO_FRAME_SAMPLE_COUNT, AUDIO_CHANNELS, AUDIO_SAMPLE_RATE);
}

/**
 * @brief Called on the captureTimer events to capture audio
 */
void OpenAL2::doAudio()
{
    QMutexLocker lock(&audioLock);

    // output section
    if(echoCancelSupported && outputInitialized && !peerSources.isEmpty()) {
        doOutput();
    } else {
        kill_filter_audio(filterer);
        filterer = nullptr;
    }

    // input section
    if (alInDev && inSubscriptions) {
        doInput();
    }
}

/**
 * @brief Returns true if the output device is open
 */
bool OpenAL2::isOutputReady() const
{
    QMutexLocker locker(&audioLock);
    return alOutDev && outputInitialized;
}

QStringList OpenAL2::outDeviceNames()
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

QStringList OpenAL2::inDeviceNames()
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

void OpenAL2::subscribeOutput(uint& sid)
{
    QMutexLocker locker(&audioLock);

    if (!autoInitOutput()) {
        qWarning("Failed to subscribe to audio output device.");
        return;
    }

    if (!alcMakeContextCurrent(alProxyContext)) {
        qWarning("Failed to activate output context.");
        return;
    }

    alGenSources(1, &sid);
    assert(sid);
    peerSources << sid;

    qDebug() << "Audio source" << sid << "created. Sources active:" << peerSources.size();
}

void OpenAL2::unsubscribeOutput(uint& sid)
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

void OpenAL2::startLoop()
{
    QMutexLocker locker(&audioLock);
    alSourcei(alMainSource, AL_LOOPING, AL_TRUE);
}

void OpenAL2::stopLoop()
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

qreal OpenAL2::inputGain() const
{
    return gain;
}

qreal OpenAL2::inputGainFactor() const
{
    return gainFactor;
}

void OpenAL2::setInputGain(qreal dB)
{
    gain = qBound(minInGain, dB, maxInGain);
    gainFactor = qPow(10.0, (gain / 20.0));
}
