/*
    Copyright © 2014-2018 by The qTox Project Contributors

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


#ifndef OPENAL_H
#define OPENAL_H

#include "src/audio/audio.h"

#include <atomic>
#include <cmath>

#include <QMutex>
#include <QObject>
#include <QTimer>

#include <cassert>

#include <al.h>
#include <alc.h>

#ifndef ALC_ALL_DEVICES_SPECIFIER
// compatibility with older versions of OpenAL
#include <alext.h>
#endif

class OpenAL : public Audio
{
    Q_OBJECT

public:
    OpenAL();
    virtual ~OpenAL();

    qreal maxOutputVolume() const { return 1; }
    qreal minOutputVolume() const { return 0; }
    qreal outputVolume() const;
    void setOutputVolume(qreal volume);

    qreal minInputGain() const;
    void setMinInputGain(qreal dB);

    qreal maxInputGain() const;
    void setMaxInputGain(qreal dB);

    qreal inputGain() const;
    void setInputGain(qreal dB);

    qreal minInputThreshold() const;
    qreal maxInputThreshold() const;

    qreal getInputThreshold() const;
    void setInputThreshold(qreal normalizedThreshold);

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
    void stopActive();

    void playAudioBuffer(uint sourceId, const int16_t* data, int samples, unsigned channels,
                         int sampleRate);

protected:
    static void checkAlError() noexcept;
    static void checkAlcError(ALCdevice* device) noexcept;

    qreal inputGainFactor() const;
    virtual void cleanupInput();
    virtual void cleanupOutput();

    bool autoInitInput();
    bool autoInitOutput();

    bool initInput(const QString& deviceName, uint32_t channels);

    void doAudio();

    virtual void doInput();
    virtual void doOutput();
    virtual void captureSamples(ALCdevice* device, int16_t* buffer, ALCsizei samples);

private:
    virtual bool initInput(const QString& deviceName);
    virtual bool initOutput(const QString& outDevDescr);
    void playMono16SoundCleanup();
    float getVolume();

protected:
    QThread* audioThread;
    mutable QMutex audioLock;

    ALCdevice* alInDev = nullptr;
    quint32 inSubscriptions = 0;
    QTimer captureTimer, playMono16Timer;

    ALCdevice* alOutDev = nullptr;
    ALCcontext* alOutContext = nullptr;
    ALuint alMainSource = 0;
    ALuint alMainBuffer = 0;
    bool outputInitialized = false;

    QList<ALuint> peerSources;
    int channels = 0;
    qreal gain = 0;
    qreal gainFactor = 1;
    qreal minInGain = -30;
    qreal maxInGain = 30;
    qreal inputThreshold = 0;
    qreal voiceHold = 250;
    bool isActive = false;
    QTimer voiceTimer;
    const qreal minInThreshold = 0.0;
    const qreal maxInThreshold = 0.4;
    int16_t* inputBuffer = nullptr;
};

#endif // OPENAL_H
