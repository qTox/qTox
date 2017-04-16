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


#ifndef OPENAL_H
#define OPENAL_H

#include "src/audio/audio.h"

#include <atomic>
#include <cmath>

#include <QMutex>
#include <QObject>
#include <QTimer>

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

class OpenAL : public Audio
{
    Q_OBJECT

public:
    OpenAL();
    ~OpenAL();

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
    void doCapture();
    qreal inputGainFactor() const;

private:
    QThread* audioThread;
    mutable QMutex audioLock;

    ALCdevice* alInDev;
    quint32 inSubscriptions;
    QTimer captureTimer, playMono16Timer;

    ALCdevice* alOutDev;
    ALCcontext* alOutContext;
    ALuint alMainSource;
    ALuint alMainBuffer;
    bool outputInitialized;

    QList<ALuint> outSources;
    qreal gain;
    qreal gainFactor;
    qreal minInGain = -30;
    qreal maxInGain = 30;
};

#endif // OPENAL_H
