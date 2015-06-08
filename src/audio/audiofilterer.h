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
    explicit AudioFilterer(const AudioFilterer&) = delete;
    ~AudioFilterer();
    AudioFilterer operator=(const AudioFilterer) = delete;
    void startFilter(unsigned int fs);
    void closeFilter();

    /* Enable/disable filters. 1 to enable, 0 to disable. */
    bool enableDisableFilters(int echo, int noise, int gain, int vad);

    bool filterAudio(int16_t* data, int samples);

    /* Give the audio output from your software to this function so it knows what echo to cancel from the frame */
    bool passAudioOutput(const int16_t *data, int samples);

    /* Tell the echo canceller how much time in ms it takes for audio to be played and recorded back after. */
    bool setEchoDelayMs(int16_t msInSndCardBuf);

private:
    struct Filter_Audio* filter{nullptr};
};

#endif // AUDIOFILTERER_H
#endif // QTOX_FILTER_AUDIO
