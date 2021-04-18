#ifndef LP_WAVETABLES_H
#define LP_WAVETABLES_H

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

typedef struct wavetable_t {
    size_t length;
    double* data;
} wavetable_t;
    
typedef struct wavetable_factory_t {
    buffer_t* (*wt)(char* name, size_t length);
    buffer_t* (*win)(char* name, size_t length);
} wavetable_factory_t;


/* forward declarations */
buffer_t* create_wavetable(char* name, size_t length);
buffer_t* create_window(char* name, size_t length);



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
    int i;
    assert(length > 1);
    for(i=0; i < length; i++) {
        out[i] = 0.5 - 0.5 * cos(2.0 * PI * i / (length-1.0));
    }
}


/* Create a wavetable stack from a string literal */
void parsewts(buffer_t** wts, char* str, int numwts, int tablesize) {
    char sep[] = ",";
    char* name = strtok(str, sep);
    int i = 0;
    while(name != NULL) {
        wts[i] = create_wavetable(name, tablesize);
        name = strtok(NULL, sep);
        i += 1;
    }
}

/* Create a window stack from a string literal */
void parsewins(buffer_t** wins, char* str, int numwins, int tablesize) {
    char sep[] = ",";
    char* name = strtok(str, sep);
    int i = 0;
    while(name != NULL) {
        wins[i] = create_window(name, tablesize);
        name = strtok(NULL, sep);
        i += 1;
    }
}

/* Create a burst table from a string literal */
void parseburst(int* burst, char* str, int numbursts) {
    char sep[] = ",";
    char* name = strtok(str, sep);
    int i = 0;

    burst[i] = atoi(name);
    while(name != NULL) {
        name = strtok(NULL, sep);
        if(name != NULL) burst[i] = atoi(name);
        i += 1;
    }
}

/* create a wavetable (-1 to 1) */
buffer_t* create_wavetable(char* name, size_t length) {
    buffer_t* buf = init_buffer(length, 1, -1);
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

/* create a window (0 to 1) */
buffer_t* create_window(char* name, size_t length) {
    buffer_t* buf = init_buffer(length, 1, -1);
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

const wavetable_factory_t Wavetable = { create_wavetable, create_window };

#endif
