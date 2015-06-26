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
#include <cmath>

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

    qreal getOutputVolume();
    void setOutputVolume(qreal volume);

    void setInputVolume(qreal volume);

    void subscribeInput();
    void unsubscribeInput();
    void openInput(const QString& inDevDescr);
    bool openOutput(const QString& outDevDescr);

    bool isInputReady();
    bool isOutputClosed();

    void playMono16Sound(const QByteArray& data);
    bool tryCaptureSamples(uint8_t* buf, int framesize);

    static void playGroupAudioQueued(void *, int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate, void*);

#ifdef QTOX_FILTER_AUDIO
    static void getEchoesToFilter(AudioFilterer* filter, int framesize);
    // is a null op #ifndef ALC_LOOPBACK_CAPTURE_SAMPLES
#endif

public slots:
    void closeInput();
    void closeOutput();
    void playGroupAudio(int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate);

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

    struct DefaultSettings {
        static constexpr int sampleRate = 48000;
        static constexpr int frameDuration = 20;
        static constexpr int audioChannels = 1;
    };
};

#endif // AUDIO_H
