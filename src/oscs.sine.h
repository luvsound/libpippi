#ifndef LP_SINEOSC_H
#define LP_SINEOSC_H

#include "pippicore.h"

typedef struct lpsineosc_t {
    lpfloat_t phase;
    lpfloat_t freq;
    lpfloat_t samplerate;
} lpsineosc_t;

typedef struct lpsineosc_factory_t {
    lpsineosc_t * (*create)(void);
    lpfloat_t (*process)(lpsineosc_t *);
    lpbuffer_t * (*render)(lpsineosc_t*, size_t, lpbuffer_t *, lpbuffer_t *, int);
    void (*destroy)(lpsineosc_t *);
} lpsineosc_factory_t;

extern const lpsineosc_factory_t LPSineOsc;

#endif
