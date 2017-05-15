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


#ifndef OPENAL2_H
#define OPENAL2_H

#include "src/audio/audio.h"

#include <atomic>
#include <cmath>

#include <QMutex>
#include <QObject>
#include <QTimer>

#include <cassert>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

extern "C" {
#include <filter_audio.h>
}

class OpenAL2 : public Audio
{
    Q_OBJECT

public:
    OpenAL2();
    ~OpenAL2();

    qreal outputVolume() const;
    void setOutputVolume(qreal volume);

    qreal minInputGain() const;
    void setMinInputGain(qreal dB);

    qreal maxInputGain() const;
    void setMaxInputGain(qreal dB);

    qreal inputGain() const;
    void setInputGain(qreal dB);

    void reinitInput(const QString& inDevDesc);
    bool reinitOutput(const QString& outDevDesc);

    bool isOutputReady() const;

    QStringList outDeviceNames();
    QStringList inDeviceNames();

    void subscribeOutput(uint& sourceId);
    void unsubscribeOutput(uint& sourceId);

    void subscribeInput();
    void unsubscribeInput();

    void startLoop();
    void stopLoop();
    void playMono16Sound(const QByteArray& data);
    void playMono16Sound(const QString& path);

    void playAudioBuffer(uint sourceId, const int16_t* data, int samples, unsigned channels,
                         int sampleRate);

signals:
    void frameAvailable(const int16_t* pcm, size_t sample_count, uint8_t channels,
                        uint32_t sampling_rate);

private:
    static void checkAlError() noexcept;
    static void checkAlcError(ALCdevice* device) noexcept;

    bool autoInitInput();
    bool autoInitOutput();
    bool initInput(const QString& deviceName);
    bool initOutput(const QString& outDevDescr);
    void cleanupInput();
    void cleanupOutput();
    void playMono16SoundCleanup();
    void doAudio();
    qreal inputGainFactor() const;
    void doInput();
    void doOutput();
    bool loadOpenALExtensions(ALCdevice* dev);
    bool initOutputEchoCancel();

private:
    QThread* audioThread;
    mutable QMutex audioLock;

    ALCdevice* alInDev;
    quint32 inSubscriptions;
    QTimer captureTimer, playMono16Timer;

    ALCdevice* alOutDev;
    ALCcontext* alOutContext;
    ALCdevice* alProxyDev;
    ALCcontext* alProxyContext;
    ALuint alMainSource;
    ALuint alMainBuffer;
    ALuint alProxySource;
    ALuint alProxyBuffer;
    bool outputInitialized;
    bool echoCancelSupported = false;

    QList<ALuint> peerSources;
    qreal gain;
    qreal gainFactor;
    qreal minInGain = -30;
    qreal maxInGain = 30;
    Filter_Audio* filterer = nullptr;
    LPALCLOOPBACKOPENDEVICESOFT alcLoopbackOpenDeviceSOFT = nullptr;
    LPALCISRENDERFORMATSUPPORTEDSOFT alcIsRenderFormatSupportedSOFT = nullptr;

    // needed because Ubuntu 14.04 lacks the AL_SOFT_source_latency extension
#ifndef AL_SOFT_source_latency
    #define AL_SAMPLE_OFFSET_LATENCY_SOFT            0x1200
    #define AL_SEC_OFFSET_LATENCY_SOFT               0x1201
    typedef void (AL_APIENTRY*LPALGETSOURCEDVSOFT)(ALuint,ALenum,const ALdouble*);
#endif

    LPALGETSOURCEDVSOFT alGetSourcedvSOFT = nullptr;
    LPALCRENDERSAMPLESSOFT alcRenderSamplesSOFT = nullptr;
};

#endif // OPENAL2_H
