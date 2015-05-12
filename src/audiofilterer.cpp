/*
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

bool AudioFilterer::filterAudio(int16_t* data, int framesize)
{
    return filter && 0 == filter_audio(filter, (int16_t*) data, framesize);
}

/* Enable/disable filters. 1 to enable, 0 to disable. */
bool AudioFilterer::enableDisableFilters(int echo, int noise, int gain)
{
    return filter && 0 == enable_disable_filters(filter, echo, noise, gain);
}

/* Give the audio output from your software to this function so it knows what echo to cancel from the frame */
bool AudioFilterer::passAudioOutput(const int16_t *data, int samples)
{
    return filter && 0 == pass_audio_output(filter, data, samples);
}

/* Tell the echo canceller how much time in ms it takes for audio to be played and recorded back after. */
bool AudioFilterer::setEchoDelayMs(int16_t msInSndCardBuf)
{
    return filter && 0 == set_echo_delay_ms(filter, msInSndCardBuf);
}

AudioFilterer::~AudioFilterer()
{
    closeFilter();
}

#endif // QTOX_FILTER_AUDIO
