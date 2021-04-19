#ifndef LP_WAVETABLE_H
#define LP_WAVETABLE_H

#include "pippicore.h"

   
typedef struct wavetable_factory_t {
    buffer_t* (*create)(char* name, size_t length);
    void (*destroy)(buffer_t*);
} wavetable_factory_t;

extern const wavetable_factory_t Wavetable;

#endif
