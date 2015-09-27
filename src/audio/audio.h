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

#include <QObject>
#include <QHash>
#include <atomic>

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/al.h>
 #include <AL/alc.h>
#endif

class QString;
class QByteArray;
class QTimer;
class QThread;
class QMutex;
struct Tox;
class AudioFilterer;

// Public default audio settings
static constexpr uint32_t AUDIO_SAMPLE_RATE = 48000; ///< The next best Opus would take is 24k
static constexpr uint32_t AUDIO_FRAME_DURATION = 20; ///< In milliseconds
static constexpr uint32_t AUDIO_FRAME_SAMPLE_COUNT = AUDIO_FRAME_DURATION * AUDIO_SAMPLE_RATE/1000;
static constexpr uint32_t AUDIO_CHANNELS = 2; ///< Ideally, we'd auto-detect, but that's a sane default

class Audio : QObject
{
    Q_OBJECT

public:
    static Audio& getInstance(); ///< Returns the singleton's instance. Will construct on first call.

    static float getOutputVolume(); ///< Returns the current output volume, between 0 and 1
    static void setOutputVolume(float volume); ///< The volume must be between 0 and 1

    static void suscribeInput(); ///< Call when you need to capture sound from the open input device.
    static void unsuscribeInput(); ///< Call once you don't need to capture on the open input device anymore.

    static void openInput(const QString& inDevDescr); ///< Open an input device, use before suscribing
    static void openOutput(const QString& outDevDescr); ///< Open an output device

    static void closeInput(); ///< Close an input device, please don't use unless everyone's unsuscribed
    static void closeOutput(); ///< Close an output device

    static bool isInputReady(); ///< Returns true if the input device is open and suscribed to
    static bool isOutputClosed(); ///< Returns true if the output device is open

    static void playMono16Sound(const QByteArray& data); ///< Play a 44100Hz mono 16bit PCM sound
    static bool tryCaptureSamples(int16_t *buf, int samples); ///< Does nothing and return false on failure

    static void playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate);

    /// May be called from any thread, will always queue a call to playGroupAudio
    /// The first and last argument are ignored, but allow direct compatibility with toxcore
    static void playGroupAudioQueued(void *, int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate, void*);

#ifdef QTOX_FILTER_AUDIO
    static void getEchoesToFilter(AudioFilterer* filter, int framesize);
    // is a null op #ifndef ALC_LOOPBACK_CAPTURE_SAMPLES
#endif

public slots:
    /// Must be called from the audio thread, plays a group call's received audio
    void playGroupAudio(int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate);

private:
    explicit Audio()=default;
    ~Audio();

private:
    static Audio* instance;
    static std::atomic<int> userCount;
    static ALCdevice* alOutDev, *alInDev;
    static QMutex* audioInLock, *audioOutLock;
    static float outputVolume;
    static ALuint alMainSource;
    static QThread* audioThread;
    static ALCcontext* alContext;
};

#endif // AUDIO_H
