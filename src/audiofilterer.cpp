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
