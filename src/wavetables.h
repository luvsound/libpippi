#ifndef LP_WAVETABLES_H
#define LP_WAVETABLES_H

#include <math.h>
#include <assert.h>

/* Wavetable generators
 * 
 * All these functions return a table of values 
 * of the given length with values between -1 and 1
 */
void wavetable_sine(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = sin((i/(lpfloat_t)length) * PI * 2.0);         
    }
}

void wavetable_square(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        if(i < (length/2.0)) {
            out[i] = 0.9999;
        } else {
            out[i] = -0.9999;
        }
    }
}

void wavetable_tri(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = fabs((i/(lpfloat_t)length) * 2.0 - 1.0) * 2.0 - 1.0;      
    }
}



/* Window generators
 *
 * All these functions return a table of values 
 * of the given length with values between 0 and 1
 */
void window_phasor(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = i/(lpfloat_t)length;      
    }
}

void window_tri(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = fabs((i/(lpfloat_t)length) * 2.0 - 1.0);      
    }
}

void window_sine(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = sin((i/(lpfloat_t)length) * PI);         
    }
}

void window_hanning(lpfloat_t* out, int length) {
    assert(length > 1);
    int i;
    for(i=0; i < length; i++) {
        out[i] = 0.5 - 0.5 * cos(2.0 * PI * i / (length-1.0));
    }
}

#endif
