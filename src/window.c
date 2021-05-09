#include "window.h"


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
#ifdef LP_FLOAT
        out[i] = fabsf((i/(lpfloat_t)length) * 2.0 - 1.0);      
#else
        out[i] = fabs((i/(lpfloat_t)length) * 2.0 - 1.0);      
#endif
    }
}

void window_sine(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
#ifdef LP_FLOAT
        out[i] = sin((i/(lpfloat_t)length) * PI);         
#else
        out[i] = sinf((i/(lpfloat_t)length) * PI);         
#endif
    }
}

void window_hanning(lpfloat_t* out, int length) {
    int i;
    assert(length > 1);
    for(i=0; i < length; i++) {
#ifdef LP_FLOAT
        out[i] = 0.5 - 0.5 * cosf(2.0 * PI * i / (length-1.0));
#else
        out[i] = 0.5 - 0.5 * cos(2.0 * PI * i / (length-1.0));
#endif
    }
}


/* create a window (0 to 1) */
buffer_t* create_window(char* name, size_t length) {
    buffer_t* buf = Buffer.create(length, 1, -1);
    if(strcmp(name, SINE) == 0) {
        window_sine(buf->data, length);            
    } else if (strcmp(name, TRI) == 0) {
        window_tri(buf->data, length);            
    } else if (strcmp(name, PHASOR) == 0) {
        window_phasor(buf->data, length);            
    } else if (strcmp(name, HANN) == 0) {
        window_hanning(buf->data, length);            
    } else {
        window_sine(buf->data, length);            
    }
    return buf;
}


void destroy_window(buffer_t* buf) {
    Buffer.destroy(buf);
}

const window_factory_t Window = { create_window, destroy_window };
