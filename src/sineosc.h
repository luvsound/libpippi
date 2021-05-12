#ifndef LP_SINEOSC_H
#define LP_SINEOSC_H

#include "pippicore.h"

typedef struct sineosc_t {
    lpfloat_t phase;
    lpfloat_t freq;
    lpfloat_t samplerate;
} sineosc_t;

typedef struct sineosc_factory_t {
    sineosc_t * (*create)(void);
    lpfloat_t (*process)(sineosc_t *);
    buffer_t * (*render)(sineosc_t*, size_t, buffer_t *, buffer_t *, int);
    void (*destroy)(sineosc_t *);
} sineosc_factory_t;

extern const sineosc_factory_t SineOsc;

#endif
