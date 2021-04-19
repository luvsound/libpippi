#ifndef LP_PULSAR_H
#define LP_PULSAR_H

#include "pippicore.h"

typedef struct pulsar_t {
    buffer_t** wts;   /* Wavetable stack */
    buffer_t** wins;  /* Window stack */
    int numwts;    /* Number of wts in stack */
    int numwins;   /* Number of wins in stack */

    buffer_t* mod;   /* Pulsewidth modulation table */
    buffer_t* morph; /* Morph table */
    int* burst;    /* Burst table */

    int tablesize; /* All tables should be this size */

    lpfloat_t samplerate;

    int boundry;
    int morphboundry;
    int burstboundry;
    int burstphase;

    lpfloat_t phase;
    lpfloat_t modphase;
    lpfloat_t morphphase;
    lpfloat_t freq;
    lpfloat_t modfreq;
    lpfloat_t morphfreq;
    lpfloat_t inc;
} pulsar_t;

typedef struct pulsar_factory_t {
    pulsar_t* (*create)(void);
    void (*destroy)(pulsar_t*);
    lpfloat_t (*process)(pulsar_t*);
} pulsar_factory_t;

extern const pulsar_factory_t Pulsar;


#endif
