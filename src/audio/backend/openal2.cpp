/*
    Copyright © 2017-2018 by The qTox Project Contributors

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
#include "src/persistence/settings.h"

#include <QDebug>
#include <QFile>
#include <QMutexLocker>
#include <QPointer>
#include <QThread>
#include <QWaitCondition>
#include <QtMath>

#include <cassert>

extern "C"
{
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
 */

static const unsigned int PROXY_BUFFER_COUNT = 4;

#define GET_PROC_ADDR(dev, name) name = reinterpret_cast<LP##name>(alcGetProcAddress(dev, #name))

typedef LPALCLOOPBACKOPENDEVICESOFT LPalcLoopbackOpenDeviceSOFT;
typedef LPALCISRENDERFORMATSUPPORTEDSOFT LPalcIsRenderFormatSupportedSOFT;
typedef LPALGETSOURCEDVSOFT LPalGetSourcedvSOFT;
typedef LPALCRENDERSAMPLESSOFT LPalcRenderSamplesSOFT;


OpenAL2::OpenAL2()
    : alProxyDev{nullptr}
    , alProxyContext{nullptr}
    , alProxySource{0}
    , alProxyBuffer{0}
{}

bool OpenAL2::initInput(const QString& deviceName)
{
    return OpenAL::initInput(deviceName, 1);
}

/**
 * @brief Loads the OpenAL extension methods needed for echo cancellation
 * @param dev OpenAL device used for the output
 * @return True when functions successfully loaded, false otherwise
 */
bool OpenAL2::loadOpenALExtensions(ALCdevice* dev)
{
    // load OpenAL extension functions
    GET_PROC_ADDR(dev, alcLoopbackOpenDeviceSOFT);
    checkAlcError(dev);
    if (!alcLoopbackOpenDeviceSOFT) {
        qDebug() << "Failed to load alcLoopbackOpenDeviceSOFT function!";
        return false;
    }

    GET_PROC_ADDR(dev, alcIsRenderFormatSupportedSOFT);
    checkAlcError(dev);
    if (!alcIsRenderFormatSupportedSOFT) {
        qDebug() << "Failed to load alcIsRenderFormatSupportedSOFT function!";
        return false;
    }

    GET_PROC_ADDR(dev, alGetSourcedvSOFT);
    checkAlcError(dev);
    if (!alGetSourcedvSOFT) {
        qDebug() << "Failed to load alGetSourcedvSOFT function!";
        return false;
    }

    GET_PROC_ADDR(dev, alcRenderSamplesSOFT);
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

    alProxyDev = alcLoopbackOpenDeviceSOFT(nullptr);
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
    assert(sinks.empty());

    outputInitialized = false;
    if (!Settings::getInstance().getAudioOutDevEnabled()) {
        return false;
    }

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

    // init master volume
    alListenerf(AL_GAIN, Settings::getInstance().getOutVolume() * 0.01f);
    checkAlError();

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
    OpenAL::cleanupOutput();

    if (echoCancelSupported) {
        alcMakeContextCurrent(alOutContext);
        alSourceStop(alProxySource);
        ALint processed = 0;
        ALuint bufids[PROXY_BUFFER_COUNT];
        alGetSourcei(alProxySource, AL_BUFFERS_PROCESSED, &processed);
        alSourceUnqueueBuffers(alProxySource, processed, bufids);
        alDeleteBuffers(processed, bufids);
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
    if (!echoCancelSupported) {
        kill_filter_audio(filterer);
        filterer = nullptr;
    }

    alcMakeContextCurrent(alOutContext);
    ALuint bufids[PROXY_BUFFER_COUNT];
    ALint processed = 0, queued = 0;
    alGetSourcei(alProxySource, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(alProxySource, AL_BUFFERS_QUEUED, &queued);

    if (processed > 0) {
        // unqueue all processed buffers
        alSourceUnqueueBuffers(alProxySource, processed, bufids);
        // delete all but the first buffer, reuse first for new data
        alDeleteBuffers(processed - 1, bufids + 1);
    } else if (queued < static_cast<ALint>(PROXY_BUFFER_COUNT)) {
        // create new buffer until the maximum is reached
        alGenBuffers(1, bufids);
    } else {
        alcMakeContextCurrent(alProxyContext);
        return;
    }

    ALdouble latency[2] = {0};
    if (echoCancelSupported) {
        alGetSourcedvSOFT(alProxySource, AL_SEC_OFFSET_LATENCY_SOFT, latency);
    }

    checkAlError();

    ALshort outBuf[AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL] = {0};
    if (echoCancelSupported) {
        alcMakeContextCurrent(alProxyContext);
        alcRenderSamplesSOFT(alProxyDev, outBuf, AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL);
        checkAlcError(alProxyDev);

        alcMakeContextCurrent(alOutContext);
    }

    alBufferData(bufids[0], AL_FORMAT_MONO16, outBuf, AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL * 2,
                 AUDIO_SAMPLE_RATE);

    alSourceQueueBuffers(alProxySource, 1, bufids);

    // initialize echo canceler if supported
    if (echoCancelSupported && !filterer) {
        filterer = new_filter_audio(AUDIO_SAMPLE_RATE);
        int16_t filterLatency = latency[1] * 1000 * 2 + AUDIO_FRAME_DURATION;
        qDebug() << "Setting filter delay to: " << filterLatency << "ms";
        set_echo_delay_ms(filterer, filterLatency);
        enable_disable_filters(filterer, 1, 1, 1, 0);
    }

    // do echo cancel
    pass_audio_output(filterer, outBuf, AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL);

    ALint state;
    alGetSourcei(alProxySource, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        qDebug() << "Proxy source underflow detected";
        alSourcePlay(alProxySource);
    }
    alcMakeContextCurrent(alProxyContext);
}

void OpenAL2::captureSamples(ALCdevice* device, int16_t* buffer, ALCsizei samples)
{
    alcCaptureSamples(device, buffer, samples);
    if (echoCancelSupported && filterer) {
        filter_audio(filterer, buffer, samples);
    }
}
