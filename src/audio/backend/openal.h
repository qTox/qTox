/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "src/audio/iaudiocontrol.h"
#include "src/audio/backend/alsink.h"
#include "src/audio/backend/alsource.h"

#include <memory>
#include <unordered_set>

#include <atomic>
#include <cmath>

#include <QMutex>
#include <QObject>
#include <QTimer>

#include <cassert>

#include <AL/al.h>
#include <AL/alc.h>

#ifndef ALC_ALL_DEVICES_SPECIFIER
// compatibility with older versions of OpenAL
#include <AL/alext.h>
#endif

class OpenAL : public IAudioControl
{
    Q_OBJECT

public:
    OpenAL();
    virtual ~OpenAL();

    qreal maxOutputVolume() const override
    {
        return 1;
    }
    qreal minOutputVolume() const override
    {
        return 0;
    }
    qreal outputVolume() const override;
    void setOutputVolume(qreal volume) override;

    qreal minInputGain() const override;
    void setMinInputGain(qreal dB) override;

    qreal maxInputGain() const override;
    void setMaxInputGain(qreal dB) override;

    qreal inputGain() const override;
    void setInputGain(qreal dB) override;

    qreal minInputThreshold() const override;
    qreal maxInputThreshold() const override;

    qreal getInputThreshold() const override;
    void setInputThreshold(qreal normalizedThreshold) override;

    void reinitInput(const QString& inDevDesc) override;
    bool reinitOutput(const QString& outDevDesc) override;

    bool isOutputReady() const override;

    QStringList outDeviceNames() override;
    QStringList inDeviceNames() override;

    std::unique_ptr<IAudioSink> makeSink() override;
    void destroySink(AlSink& sink);

    std::unique_ptr<IAudioSource> makeSource() override;
    void destroySource(IAudioSource& source);

    void startLoop(uint sourceId);
    void stopLoop(uint sourceId);
    void playMono16Sound(AlSink& sink, const IAudioSink::Sound& sound);
    void stopActive();

    void playAudioBuffer(uint sourceId, const int16_t* data, int samples, unsigned channels,
                         int sampleRate);
signals:
    void startActive(qreal msec);

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

    void cleanupBuffers(uint sourceId);
    void cleanupSound();

    float getVolume();

protected:
    QThread* audioThread;
    mutable QMutex audioLock{QMutex::Recursive};
    QString inDev{};
    QString outDev{};

    ALCdevice* alInDev = nullptr;
    QTimer captureTimer;
    QTimer cleanupTimer;

    ALCdevice* alOutDev = nullptr;
    ALCcontext* alOutContext = nullptr;

    bool outputInitialized = false;

    // Qt containers need copy operators, so use stdlib containers
    std::unordered_set<AlSink*> sinks;
    std::unordered_set<AlSink*> soundSinks;
    std::unordered_set<IAudioSource*> sources;

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
