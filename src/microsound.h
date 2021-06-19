#ifndef LP_GRAINS_H
#define LP_GRAINS_H

#include "pippicore.h"
#include "oscs.tape.h"

typedef struct lpgrain_t {
    lpfloat_t length;
    lpfloat_t offset;

    lptapeosc_t * osc;

    lpfloat_t phase;
    lpfloat_t pan;
    lpfloat_t amp;
    lpfloat_t speed;
    lpfloat_t spread;
    lpfloat_t jitter;

    lpbuffer_t * window;
    lpfloat_t window_phase;
    lpfloat_t window_phaseinc;
} lpgrain_t;

typedef struct lpcloud_t {
    lpgrain_t ** grains;
    size_t numgrains;
    size_t maxlength;
    size_t minlength;
    lpfloat_t grainamp;
    lpbuffer_t * window;
    lpbuffer_t * current_frame;
    lpbuffer_t * rb;
} lpcloud_t;

typedef struct lpgrain_factory_t {
    lpgrain_t * (*create)(lpfloat_t, lpbuffer_t *, lpbuffer_t *);
    void (*process)(lpgrain_t *, lpbuffer_t *);
    void (*destroy)(lpgrain_t *);
} lpgrain_factory_t;

typedef struct lpcloud_factory_t {
    lpcloud_t * (*create)(int, size_t, size_t, size_t, int, int);
    void (*process)(lpcloud_t *);
    void (*destroy)(lpcloud_t *);
} lpcloud_factory_t;

extern const lpgrain_factory_t LPGrain;
extern const lpcloud_factory_t LPCloud;

#endif
