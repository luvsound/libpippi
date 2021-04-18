#ifndef LP_SOUNDBUFFER_H
#define LP_SOUNDBUFFER_H

#include "pippicore.h"

typedef struct soundbuffer_factory_t {
    buffer_t* (*create)(char* name);
    void (*destroy)(soundbuffer_t*);
    enum waveform waveform;
} soundbuffer_factory_t;

buffer_t* create_soundbuffer(size_t length, int channels) {
    buffer_t* b = (buffer_t*)calloc(lpfloat_t*, length * channels);
    b->channels = channels;
    b->length = length;
    b->samplerate = 44100.0f;

    if(path == NULL) {
        return b;
    }
}


const soundbuffer_factory_t SoundBuffer = { create_soundbuffer, destroy_soundbuffer };

#endif
