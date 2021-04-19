#include "wavetable.h"

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


/* create a wavetable (-1 to 1) */
buffer_t* create_wavetable(char* name, size_t length) {
    buffer_t* buf = Buffer.create(length, 1, -1);
    if(strcmp(name, SINE) == 0) {
        wavetable_sine(buf->data, length);            
    } else if (strcmp(name, TRI) == 0) {
        wavetable_tri(buf->data, length);            
    } else if (strcmp(name, SQUARE) == 0) {
        wavetable_square(buf->data, length);            
    } else {
        wavetable_sine(buf->data, length);            
    }
    return buf;
}

void destroy_wavetable(buffer_t* buf) {
    Buffer.destroy(buf);
}

const wavetable_factory_t Wavetable = { create_wavetable, destroy_wavetable };


