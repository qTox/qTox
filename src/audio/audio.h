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

class AudioFilterer;

// Public default audio settings
static constexpr uint32_t AUDIO_SAMPLE_RATE = 48000; ///< The next best Opus would take is 24k
static constexpr uint32_t AUDIO_FRAME_DURATION = 20; ///< In milliseconds
static constexpr uint32_t AUDIO_FRAME_SAMPLE_COUNT = AUDIO_FRAME_DURATION * AUDIO_SAMPLE_RATE/1000;
static constexpr uint32_t AUDIO_CHANNELS = 2; ///< Ideally, we'd auto-detect, but that's a sane default

class Audio : public QObject
{
    using SID = unsigned int;

    Q_OBJECT

public:
    static Audio& getInstance();

public:
    qreal outputVolume();
    void setOutputVolume(qreal volume);

    qreal inputVolume();
    void setInputVolume(qreal volume);

    void reinitInput(const QString& inDevDesc);
    bool reinitOutput(const QString& outDevDesc);

    bool isInputReady();
    bool isOutputReady();

    static const char* outDeviceNames();
    static const char* inDeviceNames();
    void subscribeOutput(ALuint& sid);
    void unsubscribeOutput(ALuint& sid);

    void startLoop();
    void stopLoop();
    void playMono16Sound(const QByteArray& data);
    void playMono16Sound(const QString& path);
    bool tryCaptureSamples(int16_t *buf, int samples);

    void playAudioBuffer(quint32 alSource, const int16_t *data, int samples,
                         unsigned channels, int sampleRate);

    static void playGroupAudioQueued(void *, int group, int peer, const int16_t* data,
                                     unsigned samples, uint8_t channels, unsigned sample_rate, void*);

#if defined(QTOX_FILTER_AUDIO) && defined(ALC_LOOPBACK_CAPTURE_SAMPLES)
    void getEchoesToFilter(AudioFilterer* filter, int samples);
#endif

public slots:
    void subscribeInput();
    void unsubscribeInput();
    void playGroupAudio(int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate);

signals:
    void groupAudioPlayed(int group, int peer, unsigned short volume);

private:
    Audio();
    ~Audio();

    bool autoInitInput();
    bool autoInitOutput();
    bool initInput(QString inDevDescr);
    bool initOutput(QString outDevDescr);
    void cleanupInput();
    void cleanupOutput();

private:
    QThread*            audioThread;
    QMutex              audioLock;

    ALCdevice*          alInDev;
    ALfloat             gain;
    bool                inputInitialized;
    quint32             inSubscriptions;

    ALCdevice*          alOutDev;
    ALCcontext*         alOutContext;
    ALuint              alMainSource;
    bool                outputInitialized;

    QList<ALuint>           outSources;
};

#endif // AUDIO_H
