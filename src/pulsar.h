#ifndef _LP_PULSAR_H
#define _LP_PULSAR_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <pippi.h>

typedef struct pulsar_t {
    lpfloat_t **wts;  /* Wavetable stack */
    lpfloat_t **wins; /* Window stack */
    lpfloat_t *mod;   /* Pulsewidth modulation table */
    lpfloat_t *morph; /* Morph table */
    int* burst;    /* Burst table */
    int numwts;    /* Number of wts in stack */
    int numwins;   /* Number of wins in stack */
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

pulsar_t* create_pulsar(void) {
    /* Default args */
    lpfloat_t samplerate = 44100.0;
    int tablesize = 4096;
    lpfloat_t freq = 220.0;
    lpfloat_t modfreq = 0.03;
    lpfloat_t morphfreq = 0.3;

    /* FIXME -- better defaults */
    char wts[] = "sine,square,tri,sine";
    char wins[] = "sine,hann,sine";
    char burst[] = "1,1,0,1";

    int numwts = paramcount(wts);
    int numwins = paramcount(wins);
    int numbursts = paramcount(burst);
    int i;

    pulsar_t* p = (pulsar_t*)calloc(1, sizeof(pulsar_t));

    p->wts = (lpfloat_t**)calloc(numwts, sizeof(lpfloat_t*));

    for(i=0; i < numwts; i++) {
        p->wts[i] = (lpfloat_t*)calloc(tablesize, sizeof(lpfloat_t));
    }

    p->wins = (lpfloat_t**)calloc(numwins, sizeof(lpfloat_t*));
    for(i=0; i < numwins; i++) {
        p->wins[i] = (lpfloat_t*)calloc(tablesize, sizeof(lpfloat_t));
    }

    p->burst = (int*)calloc(numbursts, sizeof(int));

    parsewts(p->wts, wts, numwts, tablesize);
    parsewins(p->wins, wins, numwins, tablesize);
    parseburst(p->burst, burst, numbursts);

    p->numwts = numwts;
    p->numwins = numwins;
    p->samplerate = samplerate;

    p->boundry = tablesize - 1;
    p->morphboundry = numwts - 1;
    p->burstboundry = numbursts - 1;
    if(p->burstboundry <= 1) p->burst = NULL; /* Disable burst for single value tables */

    p->mod = (lpfloat_t*)calloc(tablesize, sizeof(lpfloat_t));
    p->morph = (lpfloat_t*)calloc(tablesize, sizeof(lpfloat_t));
    window_sine(p->mod, tablesize);
    window_sine(p->morph, tablesize);

    p->burstphase = 0;
    p->phase = 0;
    p->modphase = 0;

    p->freq = freq;
    p->modfreq = modfreq;
    p->morphfreq = morphfreq;

    p->inc = (1.0/samplerate) * p->boundry;

    return p;
}

lpfloat_t process_pulsar(pulsar_t* p) {
    /* Get the pulsewidth and inverse pulsewidth if the pulsewidth 
     * is zero, skip everything except phase incrementing and return 
     * a zero down the line.
     */
    lpfloat_t pw = interpolate(p->mod, p->boundry, p->modphase);
    lpfloat_t ipw = 0;

    lpfloat_t sample = 0;
    lpfloat_t mod = 0;
    lpfloat_t burst = 1;
    lpfloat_t morphpos = 0;

    lpfloat_t wtmorphpos, wtmorphfrac, a, b;
    int wtmorphidx;

    lpfloat_t winmorphpos, winmorphfrac;
    int winmorphidx;

    if(pw > 0) ipw = 1.0/pw;

    if(p->burst != NULL) {
        burst = p->burst[p->burstphase];
    }

    if(ipw > 0 && burst > 0) {
        morphpos = interpolate(p->morph, p->boundry, p->morphphase);

        assert(p->numwts >= 1);
        if(p->numwts == 1) {
            /* If there is just a single wavetable in the stack, get the current value */
            sample = interpolate(p->wts[0], p->boundry, p->phase * ipw);
        } else {
            /* If there are multiple wavetables in the stack, get their values  
             * and then interpolate the value at the morph position between them.
             */
            wtmorphpos = morphpos * imax(1, p->numwts-1);
            wtmorphidx = (int)wtmorphpos;
            wtmorphfrac = wtmorphpos - wtmorphidx;
            a = interpolate(p->wts[wtmorphidx], p->boundry, p->phase * ipw);
            b = interpolate(p->wts[wtmorphidx+1], p->boundry, p->phase * ipw);
            sample = (1.0 - wtmorphfrac) * a + (wtmorphfrac * b);
        }

        assert(p->numwins >= 1);
        if(p->numwins == 1) {
            /* If there is just a single window in the stack, get the current value */
            mod = interpolate(p->wins[0], p->boundry, p->phase * ipw);
        } else {
            /* If there are multiple wavetables in the stack, get their values 
             * and then interpolate the value at the morph position between them.
             */
            winmorphpos = morphpos * imax(1, p->numwins-1);
            winmorphidx = (int)winmorphpos;
            winmorphfrac = winmorphpos - winmorphidx;
            a = interpolate(p->wins[winmorphidx], p->boundry, p->phase * ipw);
            b = interpolate(p->wins[winmorphidx+1], p->boundry, p->phase * ipw);
            mod = (1.0 - winmorphfrac) * a + (winmorphfrac * b);
        }
    }

    /* Increment the wavetable/window phase, pulsewidth/mod phase & the morph phase */
    p->phase += p->inc * p->freq;
    p->modphase += p->inc * p->modfreq;
    p->morphphase += p->inc * p->morphfreq;

    /* Increment the burst phase on pulse boundries */
    if(p->phase >= p->boundry) {
        p->burstphase += 1;
    }

    /* Prevent phase overflow by subtracting the boundries if they have been passed */
    if(p->phase >= p->boundry) p->phase -= p->boundry;
    if(p->modphase >= p->boundry) p->modphase -= p->boundry;
    if(p->morphphase >= p->boundry) p->morphphase -= p->boundry;
    if(p->burstphase >= p->burstboundry) p->burstphase -= p->burstboundry;

    /* Multiply the wavetable value by the window value */
    return sample * mod;
}

void destroy_pulsar(pulsar_t* p) {
    int i;
    for(i=0; i < p->numwts; i++) {
        free(p->wts[i]);
    }

    for(i=0; i < p->numwins; i++) {
        free(p->wins[i]);
    }

    free(p->wts);
    free(p->wins);
    free(p->mod);
    free(p->morph);
    free(p->burst);
    free(p);
}

const pulsar_factory_t Pulsar = { create_pulsar, destroy_pulsar, process_pulsar };

#endif
