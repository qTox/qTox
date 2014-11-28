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

#ifdef __cplusplus
extern "C"{
#endif

#include "filter_audio/filter_audio.h"
static struct Filter_Audio * filter;

#ifdef __cplusplus
}
#endif

void AudioFilterer::startFilter(unsigned int fs){
    if (!filter)
        kill_filter_audio(filter);
    filter = new_filter_audio(fs);
}

void AudioFilterer::closeFilter(){
    if (filter != 0)
        kill_filter_audio(filter);
}


void AudioFilterer::filterAudio(const int16_t *data, int samples){
    if (!filter)
        return;

    filter_audio(filter, (int16_t*) data, samples);
}
