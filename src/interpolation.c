#include "interpolation.h"

lpfloat_t interpolate_linear(buffer_t* buf, lpfloat_t phase) {
    lpfloat_t frac, a, b;
    size_t i;
    
    frac = phase - (int)phase;
    i = (int)phase;

    if (i >= buf->length-1 || i < 0) return 0;

    a = buf->data[i];
    b = buf->data[i+1];

    return (1.0 - frac) * a + (frac * b);
}

lpfloat_t interpolate_linear_pos(buffer_t* buf, lpfloat_t pos) {
    return interpolate_linear(buf, pos * buf->length);
}

const interpolation_factory_t Interpolation = { interpolate_linear_pos, interpolate_linear };
