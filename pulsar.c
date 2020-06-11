#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265

typedef double* (*generator)(int);

typedef struct Pulsar {
    double **wts;  // Wavetable stack
    double **wins; // Window stack
    double *mod;   // Pulsewidth modulation table
    double *morph; // Morph table FIXME, use it...
    int numwts;    // Number of wts in stack
    int numwins;   // Number of wins in stack
    int tablesize; // All tables should be this size
    int samplerate;
    int wtlength;
    int boundry;
    int morphboundry;
    double phase;
    double modphase;
    double morphphase;
    double freq;
    double modfreq;
    double morphfreq;
    double inc;
    double morphinc;
} Pulsar;


double interpolate(double* wt, int boundry, double phase, double width) {
    if(width <= 0) return 0;

    phase *= 1.0/width;

    double frac = phase - (int)phase;
    int i = (int)phase;
    double a, b;

    if (i >= boundry) return 0;

    a = wt[i];
    b = wt[i+1];

    return (1.0 - frac) * a + (frac * b);
}


double* make_sine(int length) {
    double* out = malloc(sizeof(double) * length);
    for(int i=0; i < length; i++) {
        out[i] = sin((i/(double)length) * PI * 2.0);         
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

double* make_line_win(int length) {
    double* out = malloc(sizeof(double) * length);
    for(int i=0; i < length; i++) {
        out[i] = i/(double)length;      
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

double* make_tri_win(int length) {
    double* out = malloc(sizeof(double) * length);
    for(int i=0; i < length; i++) {
        out[i] = fabs((i/(double)length) * 2.0 - 1.0);      
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


Pulsar* init(
    int tablesize, 
    int numwts, 
    int numwins, 
    double freq, 
    double modfreq, 
    double morphfreq, 
    generator* wts, 
    generator* wins, 
    int samplerate
) {
    Pulsar* p = malloc(sizeof(Pulsar));

    p->wts = malloc(sizeof(double*) * numwts);
    for(int i=0; i < numwts; i++) {
        generator g = wts[i];
        p->wts[i] = g(tablesize);
    }

    p->wins = malloc(sizeof(double*) * numwins);
    for(int i=0; i < numwins; i++) {
        generator g = wins[i];
        p->wins[i] = g(tablesize);
    }

    p->numwts = numwts;
    p->numwins = numwins;
    p->samplerate = samplerate;
    p->wtlength = tablesize;
    p->boundry = tablesize - 1;
    p->morphboundry = numwts - 1;
    p->mod = make_sine_win(tablesize);
    p->morph = make_sine_win(tablesize);
    p->phase = 0;
    p->modphase = 0;
    p->freq = freq;
    p->modfreq = modfreq;
    p->morphfreq = morphfreq;
    p->inc = (1.0/samplerate) * p->boundry;
    p->morphinc = (1.0/samplerate) * p->morphboundry;
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
    free(p);
}

double process(Pulsar* p) {
    double pw = interpolate(p->mod, p->boundry, p->modphase, 1);

    int wtidx = (int)p->morphphase;
    double wtfrac = p->morphphase - wtidx;
    double a = interpolate(p->wts[wtidx], p->boundry, p->phase, pw);
    double b = interpolate(p->wts[wtidx+1], p->boundry, p->phase, pw);
    double sample = (1.0 - wtfrac) * a + (wtfrac * b);

    double mod = interpolate(p->wins[0], p->boundry, p->phase, pw);

    p->phase += p->inc * p->freq;
    p->modphase += p->inc * p->modfreq;
    p->morphphase += p->morphinc * p->morphfreq;

    if(p->phase >= p->boundry) p->phase -= p->boundry;
    if(p->modphase >= p->boundry) p->modphase -= p->boundry;
    if(p->morphphase >= p->morphboundry) p->morphphase -= p->morphboundry;

    return sample * mod;
}

int main() {
    int channels = 2;
    int samplerate = 44100;
    int numwts = 4;
    int numwins = 2;
    int tablesize = 4096;
    int length = 44100 * 60;

    double modfreq = 0.02;
    double morphfreq = 0.1;
    double freq = 220.0;

    generator wts[4] = {
        make_sine,
        make_square, 
        make_tri, 
        make_sine
    };

    generator wins[2] = {
        make_sine_win,
        make_tri_win
    };

    FILE *out;

    Pulsar* p = init(tablesize, numwts, numwins, freq, modfreq, morphfreq, wts, wins, samplerate);
    out = fopen("out.raw", "wb");

    double sample = 0;

    for(int i=0; i < length; i++) {
        sample = process(p);
        for(int c=0; c < channels; c++) {
            fwrite(&sample, sizeof(double), 1, out);
        }
    }
    cleanup(p);

    fclose(out);
    return 0;
}


