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

#include "audiofilterer.h"

#ifdef QTOX_FILTER_AUDIO
extern "C" {
  #include <filter_audio.h>
}

AudioFilterer::~AudioFilterer()
{
    closeFilter();
}

void AudioFilterer::startFilter(unsigned int fs)
{
    closeFilter();
    filter = new_filter_audio(fs);
}

void AudioFilterer::closeFilter()
{
    if (filter)
        kill_filter_audio(filter);
    filter = nullptr;
}

void AudioFilterer::filterAudio(int16_t* data, int framesize)
{
    if (!filter)
        return;

    filter_audio(filter, (int16_t*) data, framesize);
}
#else
AudioFilterer::~AudioFilterer() {  }
void AudioFilterer::startFilter(unsigned int) {  }
void AudioFilterer::closeFilter() { }
void AudioFilterer::filterAudio(int16_t*, int) {  }
#endif

AudioFilterer* AudioFilterer::createAudioFilter()
{
#ifdef QTOX_FILTER_AUDIO
    return new AudioFilterer();
#else
    return nullptr;
#endif
}
