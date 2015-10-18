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
    static Audio& getInstance();

public:
    void startAudioThread();

    static float getOutputVolume();
    qreal GetOutputVolume();
    static void setOutputVolume(float volume);
    void SetOutputVolume(qreal volume);

    static void setInputVolume(float volume);
    void SetInputVolume(qreal volume);

    static void suscribeInput();
    void SubscribeInput();
    static void unsuscribeInput();
    void UnsubscribeInput();

    static void openInput(const QString& inDevDescr);
    void OpenInput(const QString& inDevDescr);
    static bool openOutput(const QString& outDevDescr);
    bool OpenOutput(const QString& outDevDescr);
    static void closeInput();
    void CloseInput();
    static void closeOutput();
    void CloseOutput();

    static bool isInputReady();
    bool IsInputReady();
    static bool isOutputClosed();
    bool IsOutputClosed();

    static void playMono16Sound(const QByteArray& data);
    void PlayMono16Sound(const QByteArray& data);
    static bool tryCaptureSamples(uint8_t* buf, int framesize);
    bool TryCaptureSamples(uint8_t* buf, int framesize);

    static void playGroupAudioQueued(Tox*, int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate, void*);
    void PlayGroupAudio(int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate);

#ifdef QTOX_FILTER_AUDIO
    static void getEchoesToFilter(AudioFilterer* filter, int framesize);
    // is a null op #ifndef ALC_LOOPBACK_CAPTURE_SAMPLES
#endif

public slots:
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

private:
    QThread*            audioThread;
    QMutex              audioInLock;
    QMutex              audioOutLock;
    std::atomic<int>    inputSubscriptions;
    ALCdevice*          alOutDev;
    ALCdevice*          alInDev;
    qreal               outputVolume;
    qreal               inputVolume;
    ALuint              alMainSource;
    ALCcontext*         alContext;
    QTimer*             timer;
};

#endif // AUDIO_H
