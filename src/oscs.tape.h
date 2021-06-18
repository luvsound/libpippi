#ifndef LP_TAPEOSC_H
#define LP_TAPEOSC_H

#include "pippicore.h"

typedef struct lptapeosc_t {
    lpfloat_t phase;
    lpfloat_t freq;
    lpfloat_t speed;
    lpfloat_t samplerate;
    lpbuffer_t * buf;
    lpbuffer_t * current_frame;
} lptapeosc_t;

typedef struct lptapeosc_factory_t {
    lptapeosc_t * (*create)(lpbuffer_t *);
    void (*process)(lptapeosc_t *);
    lpbuffer_t * (*render)(lptapeosc_t *, size_t, lpbuffer_t *, lpbuffer_t *, int);
    void (*destroy)(lptapeosc_t *);
} lptapeosc_factory_t;

extern const lptapeosc_factory_t LPTapeOsc;

#endif
