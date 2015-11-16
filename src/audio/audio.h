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

struct Tox;
class AudioFilterer;
class AudioMeter;
class AudioMeterListener;
class AudioPrivate;

// Public default audio settings
static constexpr uint32_t AUDIO_SAMPLE_RATE = 48000; ///< The next best Opus would take is 24k
static constexpr uint32_t AUDIO_FRAME_DURATION = 20; ///< In milliseconds
static constexpr uint32_t AUDIO_FRAME_SAMPLE_COUNT = AUDIO_FRAME_DURATION * AUDIO_SAMPLE_RATE/1000;
static constexpr uint32_t AUDIO_CHANNELS = 2; ///< Ideally, we'd auto-detect, but that's a sane default

class Audio : public QObject
{
    Q_OBJECT

public:
    typedef QList<const void*> PtrList;

public:
    static Audio& getInstance();

public:
    void startAudioThread();

    AudioMeterListener* createAudioMeterListener() const;

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
    void createSource(quint32* sid);
    void deleteSource(quint32 sid);

    void startLoop();
    void stopLoop();
    void playMono16Sound(const QByteArray& data);
    void playMono16Sound(const QString& path);
    bool tryCaptureSamples(int16_t *buf, int samples);

    void playAudioBuffer(quint32 alSource, const int16_t *data, int samples,
                         unsigned channels, int sampleRate);

    static void playGroupAudioQueued(void *, int group, int peer, const int16_t* data,
                                     unsigned samples, uint8_t channels, unsigned sample_rate, void*);

#ifdef QTOX_FILTER_AUDIO
    static void getEchoesToFilter(AudioFilterer* filter, int framesize);
    // is a null op #ifndef ALC_LOOPBACK_CAPTURE_SAMPLES
#endif

public slots:
    void subscribeInput(const void* inListener);
    void subscribeOutput(const void* outListener);
    void unsubscribeInput(const void* inListener);
    void unsubscribeOutput(const void* outListener);
    void playGroupAudio(int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate);

signals:
    void groupAudioPlayed(int group, int peer, unsigned short volume);

private:
    Audio();
    ~Audio();

private:
    static Audio* instance;

private:
    AudioPrivate* d;
};

class AudioMeterListener : public QObject
{
    Q_OBJECT
public:
    explicit AudioMeterListener(AudioMeter* measureThread);

    void start();
    void stop();

signals:
    void gainChanged(qreal newMaxGain);

private slots:
    void doListen();

private:
    bool            mActive;
    AudioMeter*     mAudioMeter;
    qreal           mMaxGain;
};

#endif // AUDIO_H
