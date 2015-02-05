/*
    Copyright (C) 2015 by Project Tox <https://tox.im>

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

#include "toxaudiofilterer.h"
extern "C" {
  #include <filter_audio.h>
}

ToxAudioFilterer::~ToxAudioFilterer()
{
    closeFilter();
}

void ToxAudioFilterer::startFilter(unsigned int fs)
{
    closeFilter();
    filter = new_filter_audio(fs);
}

void ToxAudioFilterer::closeFilter()
{
    if (filter)
        kill_filter_audio(filter);
    filter = nullptr;
}

void ToxAudioFilterer::filterAudio(int16_t* data, int framesize)
{
    if (!filter)
        return;

    filter_audio(filter, (int16_t*) data, framesize);
}
