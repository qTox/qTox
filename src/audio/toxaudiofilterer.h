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

#ifndef TOXAUDIOFILTERER_H
#define TOXAUDIOFILTERER_H

#include "audiofilterer.h"

#ifndef _FILTER_AUDIO
typedef struct Filter_Audio Filter_Audio;
#endif

class ToxAudioFilterer : public AudioFilterer
{
public:
    ToxAudioFilterer() = default;
    virtual ~ToxAudioFilterer();

    virtual void startFilter(unsigned int fs);
    virtual void filterAudio(int16_t* data, int framesize);
    virtual void closeFilter();

private:
    struct Filter_Audio* filter{nullptr};
};

#endif // TOXAUDIOFILTERER_H
