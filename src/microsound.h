#ifndef LP_GRAINS_H
#define LP_GRAINS_H

#include "pippicore.h"

typedef struct lpgrain_t {
    size_t length;
    size_t maxlength;
    size_t minlength;
    size_t offset;
    size_t startpos;

    lpfloat_t phase;
    lpfloat_t phaseinc;
    lpfloat_t pan;
    lpfloat_t amp;
    lpfloat_t speed;

    buffer_t * window;
    lpfloat_t window_phase;
    lpfloat_t window_phaseinc;
} lpgrain_t;

typedef struct lpcloud_t {
    lpgrain_t ** grains;
    size_t numgrains;
    lpfloat_t grainamp;
    buffer_t * window;
    buffer_t * current_frame;
    buffer_t * rb;
} lpcloud_t;

typedef struct grain_factory_t {
    lpgrain_t * (*create)(size_t, size_t, buffer_t *);
    void (*process)(lpgrain_t *, buffer_t *, buffer_t *);
    void (*destroy)(lpgrain_t *);
} grain_factory_t;

typedef struct cloud_factory_t {
    lpcloud_t * (*create)(int, size_t, size_t, size_t, int, int);
    void (*process)(lpcloud_t *);
    void (*destroy)(lpcloud_t *);
} cloud_factory_t;

extern const grain_factory_t Grain;
extern const cloud_factory_t Cloud;

#endif
