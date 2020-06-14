#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Typedefs and constants
 */
#define PI 3.14159265

typedef double* (*generator)(int);

typedef struct Pulsar {
    double **wts;  // Wavetable stack
    double **wins; // Window stack
    double *mod;   // Pulsewidth modulation table
    double *morph; // Morph table
    int* burst;    // Burst table
    int numwts;    // Number of wts in stack
    int numwins;   // Number of wins in stack
    int tablesize; // All tables should be this size
    int samplerate;
    int boundry;
    int morphboundry;
    int burstboundry;
    int burstphase;
    double phase;
    double modphase;
    double morphphase;
    double freq;
    double modfreq;
    double morphfreq;
    double inc;
} Pulsar;



/* Utilities
 *
 * Small collection of utility functions
 */
int imax(int a, int b) {
    if(a > b) {
        return a;
    } else {
        return b;
    }
}

double interpolate(double* wt, int boundry, double phase) {
    double frac = phase - (int)phase;
    int i = (int)phase;
    double a, b;

    if (i >= boundry) return 0;

    a = wt[i];
    b = wt[i+1];

    return (1.0 - frac) * a + (frac * b);
}

int paramcount(char* str) {
    int count = 1;
    int i = 0;

    while(str[i] != '\0') {
        char c = str[i];
        i += 1;
        if(c == ',') {
            count += 1;
        }
    }

    return count;
}


/* Wavetable generators
 * 
 * All these functions return a table of values 
 * of the given length with values between -1 and 1
 */
double* make_sine(int length) {
    double* out = malloc(sizeof(double) * length);
    for(int i=0; i < length; i++) {
        out[i] = sin((i/(double)length) * PI * 2.0);         
    }
    return out;
}

double* make_square(int length) {
    double* out = malloc(sizeof(double) * length);
    for(int i=0; i < length; i++) {
        if(i < (length/2.0)) {
            out[i] = 0.9999;
        } else {
            out[i] = -0.9999;
        }
    }
    return out;
}

double* make_tri(int length) {
    double* out = malloc(sizeof(double) * length);
    for(int i=0; i < length; i++) {
        out[i] = fabs((i/(double)length) * 2.0 - 1.0) * 2.0 - 1.0;      
    }
    return out;
}



/* Window generators
 *
 * All these functions return a table of values 
 * of the given length with values between 0 and 1
 */
double* make_phasor(int length) {
    double* out = malloc(sizeof(double) * length);
    for(int i=0; i < length; i++) {
        out[i] = i/(double)length;      
    }
    return out;
}

double* make_tri_win(int length) {
    double* out = malloc(sizeof(double) * length);
    for(int i=0; i < length; i++) {
        out[i] = fabs((i/(double)length) * 2.0 - 1.0);      
    }
    return out;
}

double* make_sine_win(int length) {
    double* out = malloc(sizeof(double) * length);
    for(int i=0; i < length; i++) {
        out[i] = sin((i/(double)length) * PI);         
    }
    return out;
}



/* Param parsers
 */
double** parsewts(char* str, int numwts, int tablesize) {
    char sep[] = ",";
    char* token = strtok(str, sep);
    double** wts = malloc(sizeof(double*) * numwts);
    int i = 0;
    while(token != NULL) {
        if (strcmp(token, "sine") == 0) {
            wts[i] = make_sine(tablesize);            
        } else if (strcmp(token, "tri") == 0) {
            wts[i] = make_tri(tablesize);            
        } else if (strcmp(token, "square") == 0) {
            wts[i] = make_square(tablesize);            
        } else {
            wts[i] = make_sine(tablesize);            
        }

        token = strtok(NULL, sep);
        i += 1;
    }
    return wts;
}

double** parsewins(char* str, int numwins, int tablesize) {
    char sep[] = ",";
    char* token = strtok(str, sep);
    double** wins = malloc(sizeof(double*) * numwins);
    int i = 0;
    while(token != NULL) {
        if (strcmp(token, "sine") == 0) {
            wins[i] = make_sine_win(tablesize);            
        } else if (strcmp(token, "tri") == 0) {
            wins[i] = make_tri_win(tablesize);            
        } else if (strcmp(token, "phasor") == 0) {
            wins[i] = make_phasor(tablesize);            
        } else {
            wins[i] = make_sine_win(tablesize);            
        }

        token = strtok(NULL, sep);
        i += 1;
    }
    return wins;
}

int* parseburst(char* str, int numbursts) {
    char sep[] = ",";
    char* token = strtok(str, sep);
    int* burst = malloc(sizeof(int) * numbursts);
    int i = 0;
    int t = 0;
    while(token != NULL) {
        burst[i] = atoi(token);
        token = strtok(NULL, sep);
        i += 1;
    }
    return burst;
}



/* Pulsar lifecycle functions
 *
 * init -> process -> cleanup
 */
Pulsar* init(
    int tablesize, 
    double freq, 
    double modfreq, 
    double morphfreq, 
    char* wts, 
    char* wins, 
    char* burst,
    int samplerate
) {
    int numwts = paramcount(wts);
    int numwins = paramcount(wins);
    int numbursts = paramcount(burst);

    Pulsar* p = malloc(sizeof(Pulsar));

    p->wts = parsewts(wts, numwts, tablesize);
    p->wins = parsewins(wins, numwins, tablesize);
    p->burst = parseburst(burst, numbursts);

    p->numwts = numwts;
    p->numwins = numwins;
    p->samplerate = samplerate;

    p->boundry = tablesize - 1;
    p->morphboundry = numwts - 1;
    p->burstboundry = numbursts - 1;
    if(p->burstboundry <= 1) burst = NULL; // Disable burst for single value tables

    p->mod = make_sine_win(tablesize);
    p->morph = make_sine_win(tablesize);

    p->burstphase = 0;
    p->phase = 0;
    p->modphase = 0;

    p->freq = freq;
    p->modfreq = modfreq;
    p->morphfreq = morphfreq;

    p->inc = (1.0/samplerate) * p->boundry;
}

double process(Pulsar* p) {
    // Get the pulsewidth and inverse pulsewidth if the pulsewidth 
    // is zero, skip everything except phase incrementing and return 
    // a zero down the line.
    double pw = interpolate(p->mod, p->boundry, p->modphase);
    double ipw = 0;
    if(pw > 0) ipw = 1.0/pw;

    double sample = 0;
    double mod = 0;
    double burst = 1;

    if(p->burst != NULL) {
        burst = p->burst[p->burstphase];
    }

    if(ipw > 0 && burst > 0) {
        double morphpos = interpolate(p->morph, p->boundry, p->morphphase);

        assert(p->numwts >= 1);
        if(p->numwts == 1) {
            // If there is just a single wavetable in the stack, get the current value
            sample = interpolate(p->wts[0], p->boundry, p->phase * ipw);
        } else {
            // If there are multiple wavetables in the stack, get their values 
            // and then interpolate the value at the morph position between them.
            double wtmorphpos = morphpos * imax(1, p->numwts-1);
            int wtmorphidx = (int)wtmorphpos;
            double wtmorphfrac = wtmorphpos - wtmorphidx;
            double a = interpolate(p->wts[wtmorphidx], p->boundry, p->phase * ipw);
            double b = interpolate(p->wts[wtmorphidx+1], p->boundry, p->phase * ipw);
            sample = (1.0 - wtmorphfrac) * a + (wtmorphfrac * b);
        }

        assert(p->numwins >= 1);
        if(p->numwins == 1) {
            // If there is just a single window in the stack, get the current value
            mod = interpolate(p->wins[0], p->boundry, p->phase * ipw);
        } else {
            // If there are multiple wavetables in the stack, get their values 
            // and then interpolate the value at the morph position between them.
            double winmorphpos = morphpos * imax(1, p->numwins-1);
            int winmorphidx = (int)winmorphpos;
            double winmorphfrac = winmorphpos - winmorphidx;
            double a = interpolate(p->wins[winmorphidx], p->boundry, p->phase * ipw);
            double b = interpolate(p->wins[winmorphidx+1], p->boundry, p->phase * ipw);
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

void cleanup(Pulsar* p) {
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


