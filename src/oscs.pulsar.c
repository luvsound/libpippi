#include "oscs.pulsar.h"

/* TODO these belong somewhere else... */

/* Create a wavetable stack from a string literal */
void parsewts(lpbuffer_t ** wts, char* str, int numwts, int tablesize) {
    char sep[] = ",";
    char* name = strtok(str, sep);
    int i = 0;
    while(name != NULL) {
        wts[i] = LPWavetable.create(name, tablesize);
        name = strtok(NULL, sep);
        i += 1;
    }
}

/* Create a window stack from a string literal */
void parsewins(lpbuffer_t ** wins, char* str, int numwins, int tablesize) {
    char sep[] = ",";
    char* name = strtok(str, sep);
    int i = 0;
    while(name != NULL) {
        wins[i] = LPWindow.create(name, tablesize);
        name = strtok(NULL, sep);
        i += 1;
    }
}

/* Create a burst table from a string literal */
void parseburst(int * burst, char * str, int numbursts) {
    char sep[] = ",";
    char * name = strtok(str, sep);
    int i = 0;

    burst[i] = atoi(name);
    while(name != NULL) {
        name = strtok(NULL, sep);
        if(name != NULL) burst[i] = atoi(name);
        i += 1;
    }
}

lppulsarosc_t * create_pulsarosc(void) {
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

    int numwts = 4;
    int numwins = 3;
    int numbursts = 4;

    lppulsarosc_t * p = (lppulsarosc_t *)LPMemoryPool.alloc(1, sizeof(lppulsarosc_t));

    p->wts = (lpbuffer_t **)LPMemoryPool.alloc(numwts, sizeof(lpbuffer_t *));
    p->wins = (lpbuffer_t **)LPMemoryPool.alloc(numwins, sizeof(lpbuffer_t *));
    p->burst = (int*)LPMemoryPool.alloc(numbursts, sizeof(int));

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

    p->mod = LPWindow.create("sine", tablesize);
    p->morph = LPWindow.create("sine", tablesize);

    p->burstphase = 0;
    p->phase = 0;
    p->modphase = 0;

    p->freq = freq;
    p->modfreq = modfreq;
    p->morphfreq = morphfreq;

    p->inc = (1.0/samplerate) * p->boundry;

    return p;
}

lpfloat_t process_pulsarosc(lppulsarosc_t * p) {
    /* Get the pulsewidth and inverse pulsewidth if the pulsewidth 
     * is zero, skip everything except phase incrementing and return 
     * a zero down the line.
     */
    lpfloat_t pw = LPInterpolation.linear(p->mod, p->modphase);
    lpfloat_t ipw = 0;

    lpfloat_t sample = 0;
    lpfloat_t mod = 0;
    lpfloat_t burst = 1;
    lpfloat_t morphpos = 0;

    lpfloat_t wtmorphpos, wtmorphfrac, a, b;
    int wtmorphidx, wtmorphmul;

    lpfloat_t winmorphpos, winmorphfrac;
    int winmorphidx, winmorphmul;

    if(pw > 0) ipw = 1.0/pw;

    if(p->burst != NULL) {
        burst = p->burst[p->burstphase];
    }

    if(ipw > 0 && burst > 0) {
        morphpos = LPInterpolation.linear(p->morph, p->morphphase);

        assert(p->numwts >= 1);
        if(p->numwts == 1) {
            /* If there is just a single wavetable in the stack, get the current value */
            sample = LPInterpolation.linear(p->wts[0], p->phase * ipw);
        } else {
            /* If there are multiple wavetables in the stack, get their values  
             * and then interpolate the value at the morph position between them.
             */
            wtmorphmul = p->numwts-1 > 1 ? p->numwts-1 : 1;
            wtmorphpos = morphpos * wtmorphmul;
            wtmorphidx = (int)wtmorphpos;
            wtmorphfrac = wtmorphpos - wtmorphidx;
            a = LPInterpolation.linear(p->wts[wtmorphidx], p->phase * ipw);
            b = LPInterpolation.linear(p->wts[wtmorphidx+1], p->phase * ipw);
            sample = (1.0 - wtmorphfrac) * a + (wtmorphfrac * b);
        }

        assert(p->numwins >= 1);
        if(p->numwins == 1) {
            /* If there is just a single window in the stack, get the current value */
            mod = LPInterpolation.linear(p->wins[0], p->phase * ipw);
        } else {
            /* If there are multiple wavetables in the stack, get their values 
             * and then interpolate the value at the morph position between them.
             */
            winmorphmul = p->numwins-1 > 1 ? p->numwins-1 : 1;
            winmorphpos = morphpos * winmorphmul;
            winmorphidx = (int)winmorphpos;
            winmorphfrac = winmorphpos - winmorphidx;
            a = LPInterpolation.linear(p->wins[winmorphidx], p->phase * ipw);
            b = LPInterpolation.linear(p->wins[winmorphidx+1], p->phase * ipw);
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

void destroy_pulsarosc(lppulsarosc_t* p) {
    int i;
    for(i=0; i < p->numwts; i++) {
        LPMemoryPool.free(p->wts[i]);
    }

    for(i=0; i < p->numwins; i++) {
        LPMemoryPool.free(p->wins[i]);
    }

    LPMemoryPool.free(p->wts);
    LPMemoryPool.free(p->wins);
    LPMemoryPool.free(p->mod);
    LPMemoryPool.free(p->morph);
    LPMemoryPool.free(p->burst);
    LPMemoryPool.free(p);
}


const lppulsarosc_factory_t LPPulsarOsc = { create_pulsarosc, process_pulsarosc, destroy_pulsarosc };
