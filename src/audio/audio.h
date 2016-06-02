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


#ifndef AUDIO_H
#define AUDIO_H

#include <atomic>
#include <cmath>

#include <QObject>
#include <QMutex>
#include <QTimer>

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


// Public default audio settings
static constexpr uint32_t AUDIO_SAMPLE_RATE = 48000; ///< The next best Opus would take is 24k
static constexpr uint32_t AUDIO_FRAME_DURATION = 20; ///< In milliseconds
static constexpr ALint AUDIO_FRAME_SAMPLE_COUNT = AUDIO_FRAME_DURATION * AUDIO_SAMPLE_RATE/1000;
static constexpr uint32_t AUDIO_CHANNELS = 2; ///< Ideally, we'd auto-detect, but that's a sane default

class Audio : public QObject
{
    Q_OBJECT

    class Private;

public:
    static Audio& getInstance();

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

    bool isInputReady() const;
    bool isOutputReady() const;

    static QStringList outDeviceNames();
    static QStringList inDeviceNames();
    void subscribeOutput(ALuint& sid);
    void unsubscribeOutput(ALuint& sid);
    void subscribeInput();
    void unsubscribeInput();

    void startLoop();
    void stopLoop();
    void playMono16Sound(const QByteArray& data);
    void playMono16Sound(const QString& path);

    void playAudioBuffer(ALuint alSource, const int16_t *data, int samples,
                         unsigned channels, int sampleRate);

signals:
    void groupAudioPlayed(int group, int peer, unsigned short volume);
    /// When there are input subscribers, we regularly emit captured audio frames with this signal
    /// Always connect with a blocking queued connection or a lambda, or the behavior is undefined
    void frameAvailable(const int16_t *pcm, size_t sample_count, uint8_t channels, uint32_t sampling_rate);

private:
    Audio();
    ~Audio();

    static void checkAlError() noexcept;
    static void checkAlcError(ALCdevice *device) noexcept;

    bool autoInitInput();
    bool autoInitOutput();
    bool initInput(QString inDevDescr);
    bool initOutput(QString outDevDescr);
    void cleanupInput();
    void cleanupOutput();
    /// Called after a mono16 sound stopped playing
    void playMono16SoundCleanup();
    /// Called on the captureTimer events to capture audio
    void doCapture();


private:
    Private* d;

private:
    QThread*            audioThread;
    mutable QMutex      audioLock;

    ALCdevice*          alInDev;
    quint32             inSubscriptions;
    QTimer              captureTimer, playMono16Timer;

    ALCdevice*          alOutDev;
    ALCcontext*         alOutContext;
    ALuint              alMainSource;
    ALuint              alMainBuffer;
    bool                outputInitialized;

    QList<ALuint>       outSources;
};

#endif // AUDIO_H
