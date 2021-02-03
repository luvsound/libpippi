#ifndef _LP_CORE_H
#define _LP_CORE_H

#include <string.h>
#include <math.h>

#ifdef _LP_FLOAT
typedef float lpfloat_t;
#else
typedef double lpfloat_t;
#endif

#ifndef PI
#define PI 3.14159265
#endif

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

lpfloat_t interpolate(lpfloat_t* wt, int boundry, lpfloat_t phase) {
    lpfloat_t frac = phase - (int)phase;
    int i = (int)phase;

    if (i >= boundry || i < 0) return 0;

    lpfloat_t a = wt[i];
    lpfloat_t b = wt[i+1];

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



/* Param parsers
 */
void parsewts(lpfloat_t** wts, char* str, int numwts, int tablesize) {
    char sep[] = ",";
    char* token = strtok(str, sep);
    int i = 0;
    while(token != NULL) {
        if (strcmp(token, "sine") == 0) {
            wavetable_sine(wts[i], tablesize);            
        } else if (strcmp(token, "tri") == 0) {
            wavetable_tri(wts[i], tablesize);            
        } else if (strcmp(token, "square") == 0) {
            wavetable_square(wts[i], tablesize);            
        } else {
            wavetable_sine(wts[i], tablesize);            
        }

        token = strtok(NULL, sep);
        i += 1;
    }
}

void parsewins(lpfloat_t** wins, char* str, int numwins, int tablesize) {
    char sep[] = ",";
    char* token = strtok(str, sep);
    int i = 0;
    while(token != NULL) {
        if (strcmp(token, "sine") == 0) {
            window_sine(wins[i], tablesize);            
        } else if (strcmp(token, "tri") == 0) {
            window_tri(wins[i], tablesize);            
        } else if (strcmp(token, "phasor") == 0) {
            window_phasor(wins[i], tablesize);            
        } else if (strcmp(token, "hann") == 0) {
            window_hanning(wins[i], tablesize);            
        } else {
            window_sine(wins[i], tablesize);            
        }

        token = strtok(NULL, sep);
        i += 1;
    }
}

void parseburst(int* burst, char* str, int numbursts) {
    char sep[] = ",";
    char* token = strtok(str, sep);
    int i = 0;

    burst[i] = atoi(token);
    while(token != NULL) {
        token = strtok(NULL, sep);
        if(token != NULL) burst[i] = atoi(token);
        i += 1;
    }
}

#include "wavetables.h"
#include "pulsar.h"

#endif
