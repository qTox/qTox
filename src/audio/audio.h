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
#include <QMutexLocker>
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

class Audio : public QObject
{
    Q_OBJECT

public:
    static Audio& getInstance(); ///< Returns the singleton's instance. Will construct on first call.

public:
    void startAudioThread();

    static float getOutputVolume(); ///< Returns the current output volume, between 0 and 1
    qreal GetOutputVolume();
    static void setOutputVolume(float volume); ///< The volume must be between 0 and 1
    void SetOutputVolume(qreal volume);

    static void setInputVolume(float volume); ///< The volume must be between 0 and 2
    void SetInputVolume(qreal volume);

    static void suscribeInput(); ///< Call when you need to capture sound from the open input device.
    void SubscribeInput();
    static void unsuscribeInput(); ///< Call once you don't need to capture on the open input device anymore.
    void UnSubscribeInput();

    static void openInput(const QString& inDevDescr); ///< Open an input device, use before suscribing
    void OpenInput(const QString& inDevDescr);
    static bool openOutput(const QString& outDevDescr); ///< Open an output device
    bool OpenOutput(const QString& outDevDescr);
    static void closeInput();
    void CloseInput();
    static void closeOutput(); ///< Close an output device
    void CloseOutput();

    static bool isInputReady(); ///< Returns true if the input device is open and suscribed to
    bool IsInputReady();
    static bool isOutputClosed(); ///< Returns true if the output device is open
    bool IsOutputClosed();

    static void playMono16Sound(const QByteArray& data); ///< Play a 44100Hz mono 16bit PCM sound
    void PlayMono16Sound(const QByteArray& data);
    static bool tryCaptureSamples(uint8_t* buf, int framesize); ///< Does nothing and return false on failure
    bool TryCaptureSamples(uint8_t* buf, int framesize);

    /// May be called from any thread, will always queue a call to playGroupAudio
    /// The first and last argument are ignored, but allow direct compatibility with toxcore
    static void playGroupAudioQueued(Tox*, int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate, void*);
    void PlayGroupAudio(int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate);

#ifdef QTOX_FILTER_AUDIO
    static void getEchoesToFilter(AudioFilterer* filter, int framesize);
    // is a null op #ifndef ALC_LOOPBACK_CAPTURE_SAMPLES
#endif

public slots:
    /// Must be called from the audio thread, plays a group call's received audio
    void playGroupAudio(int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate);
    static void pauseOutput();
    void PauseOutput();

signals:
    void groupAudioPlayed(int group, int peer, unsigned short volume);

private:
    Audio();
    ~Audio();

    void playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate);

private:
    static Audio* instance;

    QThread*            audioThread;
    QMutex              audioInLock;
    QMutex              audioOutLock;
    std::atomic<int>    userCount;
    ALCdevice*          alOutDev;
    ALCdevice*          alInDev;
    qreal               outputVolume;
    qreal               inputVolume;
    ALuint              alMainSource;
    ALCcontext*         alContext;
    QTimer*             timer;
};

#endif // AUDIO_H
