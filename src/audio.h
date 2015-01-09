/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
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

class Audio : QObject
{
    Q_OBJECT

public:
    static Audio& getInstance(); ///< Returns the singleton's instance. Will construct on first call.

    static void suscribeInput(); ///< Call when you need to capture sound from the open input device.
    static void unsuscribeInput(); ///< Call once you don't need to capture on the open input device anymore.

    static void openInput(const QString& inDevDescr); ///< Open an input device, use before suscribing
    static void openOutput(const QString& outDevDescr); ///< Open an output device

    static void closeInput(); ///< Close an input device, please don't use unless everyone's unsuscribed
    static void closeOutput(); ///< Close an output device

    static bool isInputReady(); ///< Returns true if the input device is open and suscribed to
    static bool isOutputClosed(); ///< Returns true if the output device is open

    static void playMono16Sound(const QByteArray& data); ///< Play a 44100Hz mono 16bit PCM sound
    static bool tryCaptureSamples(uint8_t* buf, int framesize); ///< Does nothing and return false on failure

    /// May be called from any thread, will always queue a call to playGroupAudio
    /// The first and last argument are ignored, but allow direct compatibility with toxcore
    static void playGroupAudioQueued(Tox*, int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate, void*);

public slots:
    /// Must be called from the audio thread, plays a group call's received audio
    void playGroupAudio(int group, int peer, const int16_t* data,
                        unsigned samples, uint8_t channels, unsigned sample_rate);

public:
    static QThread* audioThread;
    static ALCcontext* alContext;
    static ALuint alMainSource;

private:
    explicit Audio()=default;
    static void playAudioBuffer(ALuint alSource, const int16_t *data, int samples, unsigned channels, int sampleRate);

private:
    static Audio* instance;
    static std::atomic<int> userCount;
    static ALCdevice* alOutDev, *alInDev;
    static QMutex* audioInLock, *audioOutLock;
};

#endif // AUDIO_H
