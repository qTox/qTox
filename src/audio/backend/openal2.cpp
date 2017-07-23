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
 * The following graphic describes the audio rendering pipeline with echo canceling
 *
 * |       alProxyContext       |               |        alOutContext        |
 * peerSources[]                |               |                            |
 *              \               |               |                            |
 *               -> alProxyDev -> filter_audio -> alProxySource -> alOutDev -> Soundcard
 *              /
 * alMainSource
 *
 * Without echo cancelling the pipeline is simplified through alProxyDev = alOutDev
 * and alProxyContext = alOutContext
 *
 * |      alProxyContext      |
 * peerSources[]              |
 *              \             |
 *               -> alOutDev -> Soundcard
 *              /
 * alMainSource
 *
 * To keep all functions in writing to the correct context, all functions changing
 * the context MUST exit with alProxyContext as active context and MUST not be
 * interrupted. For this to work, all functions of the base class modifying the
 * context have to be overriden.
 *
 * @var BUFFER_COUNT
 * @brief Number of buffers to use per audio source
 */

static const unsigned int BUFFER_COUNT = 16;
static const unsigned int PROXY_BUFFER_COUNT = 4;

OpenAL2::OpenAL2()
    : alProxyDev{nullptr}
    , alProxyContext{nullptr}
    , alProxySource{0}
    , alProxyBuffer{0}
{
}

bool OpenAL2::initInput(const QString& deviceName)
{
    if (!Settings::getInstance().getAudioInDevEnabled())
        return false;

    qDebug() << "Opening audio input" << deviceName;
    assert(!alInDev);

    const ALCsizei bufSize = AUDIO_FRAME_SAMPLE_COUNT * 4 * 2;

    const QByteArray qDevName = deviceName.toUtf8();
    const ALchar* tmpDevName = qDevName.isEmpty() ? nullptr : qDevName.constData();
    alInDev = alcCaptureOpenDevice(tmpDevName, AUDIO_SAMPLE_RATE, AL_FORMAT_MONO16, bufSize);

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
    alcLoopbackOpenDeviceSOFT = reinterpret_cast<LPALCLOOPBACKOPENDEVICESOFT>(
        alcGetProcAddress(dev, "alcLoopbackOpenDeviceSOFT"));
    checkAlcError(dev);
    if (!alcLoopbackOpenDeviceSOFT) {
        qDebug() << "Failed to load alcLoopbackOpenDeviceSOFT function!";
        return false;
    }

    alcIsRenderFormatSupportedSOFT = reinterpret_cast<LPALCISRENDERFORMATSUPPORTEDSOFT>(
        alcGetProcAddress(dev, "alcIsRenderFormatSupportedSOFT"));
    checkAlcError(dev);
    if (!alcIsRenderFormatSupportedSOFT) {
        qDebug() << "Failed to load alcIsRenderFormatSupportedSOFT function!";
        return false;
    }

    alGetSourcedvSOFT =
        reinterpret_cast<LPALGETSOURCEDVSOFT>(alcGetProcAddress(dev, "alGetSourcedvSOFT"));
    checkAlcError(dev);
    if (!alGetSourcedvSOFT) {
        qDebug() << "Failed to load alGetSourcedvSOFT function!";
        return false;
    }

    alcRenderSamplesSOFT = reinterpret_cast<LPALCRENDERSAMPLESSOFT>(
        alcGetProcAddress(alOutDev, "alcRenderSamplesSOFT"));
    checkAlcError(dev);
    if (!alcRenderSamplesSOFT) {
        qDebug() << "Failed to load alcRenderSamplesSOFT function!";
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

    if (!loadOpenALExtensions(alOutDev)) {
        qDebug() << "Couldn't load needed OpenAL extensions";
        return false;
    }

    // source for proxy output
    alGenSources(1, &alProxySource);
    checkAlError();

    // configuration for the loopback device
    ALCint attrs[] = {ALC_FORMAT_CHANNELS_SOFT,
                      ALC_MONO_SOFT,
                      ALC_FORMAT_TYPE_SOFT,
                      ALC_SHORT_SOFT,
                      ALC_FREQUENCY,
                      Audio::AUDIO_SAMPLE_RATE,
                      0}; // End of List

    alProxyDev = alcLoopbackOpenDeviceSOFT(NULL);
    checkAlcError(alProxyDev);
    if (!alProxyDev) {
        qDebug() << "Couldn't create proxy device";
        alDeleteSources(1, &alProxySource); // cleanup source
        return false;
    }

    if (!alcIsRenderFormatSupportedSOFT(alProxyDev, attrs[5], attrs[1], attrs[3])) {
        qDebug() << "Unsupported format for loopback";
        alcCloseDevice(alProxyDev);         // cleanup loopback dev
        alDeleteSources(1, &alProxySource); // cleanup source
        return false;
    }

    alProxyContext = alcCreateContext(alProxyDev, attrs);
    checkAlcError(alProxyDev);
    if (!alProxyContext) {
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

    if (!echoCancelSupported) {
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

    // ensure alProxyContext is active
    alcMakeContextCurrent(alProxyContext);
    outputInitialized = true;
    return true;
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
        // TODO: delete buffers
        alcMakeContextCurrent(nullptr);
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
 * @brief Handle audio output
 */
void OpenAL2::doOutput()
{
    alcMakeContextCurrent(alOutContext);
    ALuint bufids[PROXY_BUFFER_COUNT];
    ALint processed = 0, queued = 0;
    alGetSourcei(alProxySource, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(alProxySource, AL_BUFFERS_QUEUED, &queued);

    // qDebug() << "Speedtest processed: " << processed << " queued: " << queued;

    if (processed > 0) {
        // unqueue all processed buffers
        alSourceUnqueueBuffers(alProxySource, 1, bufids);
    } else if (queued < PROXY_BUFFER_COUNT) {
        // create new buffer until the maximum is reached
        alGenBuffers(1, bufids);
    } else {
        alcMakeContextCurrent(alProxyContext);
        return;
    }

    ALdouble latency[2] = {0};
    alGetSourcedvSOFT(alProxySource, AL_SEC_OFFSET_LATENCY_SOFT, latency);
    checkAlError();
    // qDebug() << "Playback latency: " << latency[1] << "offset: " << latency[0];

    ALshort outBuf[AUDIO_FRAME_SAMPLE_COUNT] = {0};
    alcMakeContextCurrent(alProxyContext);
    alcRenderSamplesSOFT(alProxyDev, outBuf, AUDIO_FRAME_SAMPLE_COUNT);
    checkAlcError(alProxyDev);

    alcMakeContextCurrent(alOutContext);
    alBufferData(bufids[0], AL_FORMAT_MONO16, outBuf, AUDIO_FRAME_SAMPLE_COUNT * 2, AUDIO_SAMPLE_RATE);
    alSourceQueueBuffers(alProxySource, 1, bufids);

    // initialize echo canceler if supported
    if (!filterer) {
        filterer = new_filter_audio(AUDIO_SAMPLE_RATE);
        int16_t filterLatency = latency[1] * 1000 * 2 + AUDIO_FRAME_DURATION;
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
    alcMakeContextCurrent(alProxyContext);
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

    int16_t buf[AUDIO_FRAME_SAMPLE_COUNT];
    alcCaptureSamples(alInDev, buf, AUDIO_FRAME_SAMPLE_COUNT);

    int retVal = 0;
    if (echoCancelSupported && filterer) {
        retVal = filter_audio(filterer, buf, AUDIO_FRAME_SAMPLE_COUNT);
    }

    // gain amplification with clipping to 16-bit boundaries
    for (quint32 i = 0; i < AUDIO_FRAME_SAMPLE_COUNT; ++i) {
        int ampPCM = qBound<int>(std::numeric_limits<int16_t>::min(),
                                 qRound(buf[i] * OpenAL::inputGainFactor()),
                                 std::numeric_limits<int16_t>::max());

        buf[i] = static_cast<int16_t>(ampPCM);
    }

    emit Audio::frameAvailable(buf, AUDIO_FRAME_SAMPLE_COUNT, 1, AUDIO_SAMPLE_RATE);
}

/**
 * @brief Called on the captureTimer events to capture audio
 */
void OpenAL2::doAudio()
{
    QMutexLocker lock(&audioLock);

    // output section
    if (echoCancelSupported && outputInitialized && !peerSources.isEmpty()) {
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
