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


#ifdef QTOX_FILTER_AUDIO

#include "audiofilterer.h"
extern "C"{
#include <filter_audio.h>
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


AudioFilterer::~AudioFilterer()
{
    closeFilter();
}

#endif // QTOX_FILTER_AUDIO
