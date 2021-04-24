#include "interpolation.h"

lpfloat_t interpolate_hermite(buffer_t* buf, lpfloat_t phase) {
    lpfloat_t y0, y1, y2, y3, frac;
    lpfloat_t c0, c1, c2, c3;
    int i0, i1, i2, i3, boundry;

    if(buf->length == 1) return buf->data[0];
    if(buf->length < 1) return 0;

    boundry = buf->length - 1;

    frac = phase - (int)phase;
    i1 = (int)phase;
    i2 = i1 + 1;
    i3 = i2 + 1;
    i0 = i1 - 1;

    y0 = 0;
    y1 = 0;
    y2 = 0;
    y3 = 0;

    if(i0 >= 0) y0 = buf->data[i0];
    if(i1 <= boundry) y1 = buf->data[i1];
    if(i2 <= boundry) y2 = buf->data[i2];
    if(i3 <= boundry) y3 = buf->data[i3];

    /* This part was taken from version #2 by James McCartney 
     * https://www.musicdsp.org/en/latest/Other/93-hermite-interpollation.html
     */
    c0 = y1;
    c1 = 0.5 * (y2 - y0);
    c3 = 1.5 * (y1 - y2) + 0.5 * (y3 - y0);
    c2 = y0 - y1 + c1 - c3;
    return ((c3 * frac + c2) * frac + c1) * frac + c0;
}

lpfloat_t interpolate_hermite_pos(buffer_t* buf, lpfloat_t pos) {
    return interpolate_hermite(buf, pos * buf->length);
}

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

const interpolation_factory_t Interpolation = { interpolate_linear_pos, interpolate_linear, interpolate_hermite_pos, interpolate_hermite };
