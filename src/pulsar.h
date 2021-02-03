#ifndef _LP_PULSAR_H
#define _LP_PULSAR_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <pippi.h>

typedef lpfloat_t* (*generator)(int);

typedef struct pulsar_t {
    lpfloat_t **wts;  // Wavetable stack
    lpfloat_t **wins; // Window stack
    lpfloat_t *mod;   // Pulsewidth modulation table
    lpfloat_t *morph; // Morph table
    int* burst;    // Burst table
    int numwts;    // Number of wts in stack
    int numwins;   // Number of wins in stack
    int tablesize; // All tables should be this size
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

typedef struct factory_t {
    pulsar_t* (*create)(void);
    void (*destroy)(pulsar_t*);
    lpfloat_t (*process)(pulsar_t*);

    // defaults
} factory_t;

extern const factory_t Pulsar;

pulsar_t* init_pulsar(
    int tablesize, 
    lpfloat_t freq, 
    lpfloat_t modfreq, 
    lpfloat_t morphfreq, 
    char* wts, 
    char* wins, 
    char* burst,
    lpfloat_t samplerate
) {
    int numwts = paramcount(wts);
    int numwins = paramcount(wins);
    int numbursts = paramcount(burst);

    pulsar_t* p = (pulsar_t*)malloc(sizeof(pulsar_t));

    p->wts = (lpfloat_t**)malloc(sizeof(lpfloat_t*) * numwts);
    for(int i=0; i < numwts; i++) {
        p->wts[i] = (lpfloat_t*)malloc(sizeof(lpfloat_t) * tablesize);
    }

    p->wins = (lpfloat_t**)malloc(sizeof(lpfloat_t*) * numwins);
    for(int i=0; i < numwins; i++) {
        p->wins[i] = (lpfloat_t*)malloc(sizeof(lpfloat_t) * tablesize);
    }

    p->burst = (int*)malloc(sizeof(int) * numbursts);

    parsewts(p->wts, wts, numwts, tablesize);
    parsewins(p->wins, wins, numwins, tablesize);
    parseburst(p->burst, burst, numbursts);

    p->numwts = numwts;
    p->numwins = numwins;
    p->samplerate = samplerate;

    p->boundry = tablesize - 1;
    p->morphboundry = numwts - 1;
    p->burstboundry = numbursts - 1;
    if(p->burstboundry <= 1) burst = NULL; // Disable burst for single value tables

    p->mod = (lpfloat_t*)malloc(sizeof(lpfloat_t) * tablesize);
    p->morph = (lpfloat_t*)malloc(sizeof(lpfloat_t) * tablesize);
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
    // Get the pulsewidth and inverse pulsewidth if the pulsewidth 
    // is zero, skip everything except phase incrementing and return 
    // a zero down the line.
    lpfloat_t pw = interpolate(p->mod, p->boundry, p->modphase);
    lpfloat_t ipw = 0;
    if(pw > 0) ipw = 1.0/pw;

    lpfloat_t sample = 0;
    lpfloat_t mod = 0;
    lpfloat_t burst = 1;

    if(p->burst != NULL) {
        burst = p->burst[p->burstphase];
    }

    if(ipw > 0 && burst > 0) {
        lpfloat_t morphpos = interpolate(p->morph, p->boundry, p->morphphase);

        assert(p->numwts >= 1);
        if(p->numwts == 1) {
            // If there is just a single wavetable in the stack, get the current value
            sample = interpolate(p->wts[0], p->boundry, p->phase * ipw);
        } else {
            // If there are multiple wavetables in the stack, get their values 
            // and then interpolate the value at the morph position between them.
            lpfloat_t wtmorphpos = morphpos * imax(1, p->numwts-1);
            int wtmorphidx = (int)wtmorphpos;
            lpfloat_t wtmorphfrac = wtmorphpos - wtmorphidx;
            lpfloat_t a = interpolate(p->wts[wtmorphidx], p->boundry, p->phase * ipw);
            lpfloat_t b = interpolate(p->wts[wtmorphidx+1], p->boundry, p->phase * ipw);
            sample = (1.0 - wtmorphfrac) * a + (wtmorphfrac * b);
        }

        assert(p->numwins >= 1);
        if(p->numwins == 1) {
            // If there is just a single window in the stack, get the current value
            mod = interpolate(p->wins[0], p->boundry, p->phase * ipw);
        } else {
            // If there are multiple wavetables in the stack, get their values 
            // and then interpolate the value at the morph position between them.
            lpfloat_t winmorphpos = morphpos * imax(1, p->numwins-1);
            int winmorphidx = (int)winmorphpos;
            lpfloat_t winmorphfrac = winmorphpos - winmorphidx;
            lpfloat_t a = interpolate(p->wins[winmorphidx], p->boundry, p->phase * ipw);
            lpfloat_t b = interpolate(p->wins[winmorphidx+1], p->boundry, p->phase * ipw);
            mod = (1.0 - winmorphfrac) * a + (winmorphfrac * b);
        }
    }

    // Increment the wavetable/window phase, pulsewidth/mod phase & the morph phase
    p->phase += p->inc * p->freq;
    p->modphase += p->inc * p->modfreq;
    p->morphphase += p->inc * p->morphfreq;

    // Increment the burst phase on pulse boundries
    if(p->phase >= p->boundry) {
        p->burstphase += 1;
    }

    // Prevent phase overflow by subtracting the boundries if they have been passed
    if(p->phase >= p->boundry) p->phase -= p->boundry;
    if(p->modphase >= p->boundry) p->modphase -= p->boundry;
    if(p->morphphase >= p->boundry) p->morphphase -= p->boundry;
    if(p->burstphase >= p->burstboundry) p->burstphase -= p->burstboundry;

    // Multiply the wavetable value by the window value
    return sample * mod;
}

pulsar_t* create_pulsar(void) {
    int samplerate = 44100;
    int tablesize = 4096;

    lpfloat_t modfreq = 0.03;
    lpfloat_t morphfreq = 0.3;
    lpfloat_t freq = 220.0;

    char wts[] = "sine,square,tri,sine";
    char wins[] = "sine,hann,sine";
    char burst[] = "1,1,0,1";

    return init_pulsar(tablesize, freq, modfreq, morphfreq, wts, wins, burst, samplerate);
}

void destroy_pulsar(pulsar_t* p) {
    for(int i=0; i < p->numwts; i++) {
        free(p->wts[i]);
    }

    for(int i=0; i < p->numwins; i++) {
        free(p->wins[i]);
    }

    free(p->wts);
    free(p->wins);
    free(p->mod);
    free(p->morph);
    free(p->burst);
    free(p);
}

const factory_t Pulsar = {
    .create = create_pulsar, 
    .destroy = destroy_pulsar,
    .process = process_pulsar

    // defaults
};

#endif
