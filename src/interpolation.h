#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#import "pippicore.h"

typedef lpfloat_t (*interp_t)(lpfloat_t* wt, int boundry, lpfloat_t phase);

lpfloat_t interpolate(lpfloat_t* wt, int boundry, lpfloat_t phase) {
    lpfloat_t frac = phase - (int)phase;
    int i = (int)phase;

    if (i >= boundry || i < 0) return 0;

    lpfloat_t a = wt[i];
    lpfloat_t b = wt[i+1];

    return (1.0 - frac) * a + (frac * b);
}

#endif
