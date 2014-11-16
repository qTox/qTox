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

class Audio
{
public:
    static void suscribeInput(); ///< Call when you need to capture sound from the open input device.
    static void unsuscribeInput(); ///< Call once you don't need to capture on the open input device anymore.

    static void openInput(const QString& inDevDescr); ///< Open an input device, use before suscribing
    static void openOutput(const QString& outDevDescr); ///< Open an output device

    static void closeInput(); ///< Close an input device, please don't use unless everyone's unsuscribed
    static void closeOutput(); ///< Close an output device

    static void playMono16Sound(const QByteArray& data); ///< Play a 44100Hz mono 16bit PCM sound

public:
    static ALCdevice* alOutDev, *alInDev;
    static ALCcontext* alContext;
    static ALuint alMainSource;

private:
    Audio();

private:
    static std::atomic<int> userCount;
};

#endif // AUDIO_H
