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
#include <QMutexLocker>
#include <atomic>
#include <cmath>

#if defined(__APPLE__) && defined(__MACH__)
 #include <OpenAL/al.h>
 #include <OpenAL/alc.h>
#else
 #include <AL/al.h>
 #include <AL/alc.h>
 #include <AL/alext.h>
#endif

class QTimer;
struct Tox;
class AudioFilterer;

// Public default audio settings
static constexpr uint32_t AUDIO_SAMPLE_RATE = 48000; ///< The next best Opus would take is 24k
static constexpr uint32_t AUDIO_FRAME_DURATION = 20; ///< In milliseconds
static constexpr uint32_t AUDIO_FRAME_SAMPLE_COUNT = AUDIO_FRAME_DURATION * AUDIO_SAMPLE_RATE/1000;
static constexpr uint32_t AUDIO_CHANNELS = 2; ///< Ideally, we'd auto-detect, but that's a sane default

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
    void subscribeOutput();
    void unsubscribeOutput();
    void openInput(const QString& inDevDescr);
    bool openOutput(const QString& outDevDescr);

    bool isInputReady();
    bool isInputSubscribed();
    bool isOutputReady();

    static void createSource(ALuint* source);
    static void deleteSource(ALuint* source);

    void startLoop();
    void stopLoop();
    void playMono16Sound(const QByteArray& data);
    void playMono16Sound(const char* path);
    bool tryCaptureSamples(int16_t *buf, int samples);

    static void playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate);

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

    void initInput(const QString& inDevDescr);
    bool initOutput(const QString& outDevDescr);
    void cleanupInput();
    void cleanupOutput();

private:
    static Audio* instance;

private:
    QThread*            audioThread;
    QMutex              audioInLock;
    QMutex              audioOutLock;
    std::atomic<int>    inputSubscriptions;
    std::atomic<int>    outputSubscriptions;
    ALCdevice*          alOutDev;
    ALCdevice*          alInDev;
    qreal               outputVolume;
    qreal               inputVolume;
    ALuint              alMainSource;
    ALCcontext*         alContext;
    QTimer*             timer;
};

#endif // AUDIO_H
