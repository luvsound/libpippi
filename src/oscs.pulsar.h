#ifndef LP_PULSAR_H
#define LP_PULSAR_H

#include "pippicore.h"

typedef struct lppulsarosc_t {
    lpbuffer_t ** wts;   /* Wavetable stack */
    lpbuffer_t ** wins;  /* Window stack */
    int numwts;    /* Number of wts in stack */
    int numwins;   /* Number of wins in stack */

    lpbuffer_t * mod;   /* Pulsewidth modulation table */
    lpbuffer_t * morph; /* Morph table */
    int * burst;    /* Burst table */

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
} lppulsarosc_t;

typedef struct lppulsarosc_factory_t {
    lppulsarosc_t * (*create)(void);
    lpfloat_t (*process)(lppulsarosc_t *);
    void (*destroy)(lppulsarosc_t*);
} lppulsarosc_factory_t;

extern const lppulsarosc_factory_t LPPulsarOsc;

#endif
