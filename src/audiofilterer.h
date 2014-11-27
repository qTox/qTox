#ifndef AUDIOFILTERER_H
#define AUDIOFILTERER_H

#include <cstdint>

class AudioFilterer
{
public:
    static void startFilter(unsigned int fs);
    static void filterAudio(const int16_t *data, int samples);
    static void closeFilter();
};

#endif // AUDIOFILTERER_H
