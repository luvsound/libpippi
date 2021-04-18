#include "interpolation.h"

lpfloat_t interpolate_linear(lpfloat_t* wt, int boundry, lpfloat_t phase) {
    lpfloat_t frac, a, b;
    int i;
    
    frac = phase - (int)phase;
    i = (int)phase;

    if (i >= boundry || i < 0) return 0;

    a = wt[i];
    b = wt[i+1];

    return (1.0 - frac) * a + (frac * b);
}


const interpolation_factory_t Interpolation = { interpolate_linear };
