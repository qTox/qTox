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
#ifndef AUDIOFILTERER_H
#define AUDIOFILTERER_H
#include <cstdint>

#ifndef _FILTER_AUDIO
typedef struct Filter_Audio Filter_Audio;
#endif

class AudioFilterer
{
public:
    explicit AudioFilterer() = default;
    ~AudioFilterer();

    void startFilter(unsigned int fs);
    void filterAudio(int16_t* data, int framesize);
    void closeFilter();

private:
    struct Filter_Audio* filter{nullptr};
};

#endif // AUDIOFILTERER_H
#endif // QTOX_FILTER_AUDIO
