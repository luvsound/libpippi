#ifndef LP_CORE_H
#define LP_CORE_H

/* std includes */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TYPES */
#ifdef LP_FLOAT
typedef float lpfloat_t;
#else
typedef double lpfloat_t;
#endif

/* CONSTANTS */
#ifndef PI
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062
#endif

#ifndef PI2
#define PI2 (PI*2.0)
#endif

#define SINE "sine"
#define SQUARE "square"
#define TRI "tri"
#define PHASOR "phasor"
#define HANN "hann"
#define RND "rnd"

#define DEFAULT_CHANNELS 2
#define DEFAULT_SAMPLERATE 48000

/* Core datatypes */
typedef struct buffer_t {
    lpfloat_t* data;
    size_t length;
    int samplerate;
    int channels;

    /* used for different types of playback */
    lpfloat_t phase;
    size_t range;
    size_t pos;
} buffer_t;

/* Factories & static interfaces */
typedef struct lprand_t {
    lpfloat_t logistic_seed;
    lpfloat_t logistic_x;

    lpfloat_t lorenz_timestep;
    lpfloat_t lorenz_x;
    lpfloat_t lorenz_y;
    lpfloat_t lorenz_z;
    lpfloat_t lorenz_a;
    lpfloat_t lorenz_b;
    lpfloat_t lorenz_c;

    void (*seed)(int);

    lpfloat_t (*logistic)(lpfloat_t, lpfloat_t);

    lpfloat_t (*lorenz)(lpfloat_t, lpfloat_t);
    lpfloat_t (*lorenzX)(lpfloat_t, lpfloat_t);
    lpfloat_t (*lorenzY)(lpfloat_t, lpfloat_t);
    lpfloat_t (*lorenzZ)(lpfloat_t, lpfloat_t);

    lpfloat_t (*rand_base)(lpfloat_t, lpfloat_t);
    lpfloat_t (*rand)(lpfloat_t, lpfloat_t);
    int (*randint)(int, int);
    int (*randbool)(void);
    int (*choice)(int);
} lprand_t;

typedef struct buffer_factory_t {
    buffer_t * (*create)(size_t, int, int);
    void (*scale)(buffer_t *, lpfloat_t, lpfloat_t, lpfloat_t, lpfloat_t);
    lpfloat_t (*play)(buffer_t *, lpfloat_t);
    buffer_t * (*mix)(buffer_t *, buffer_t *);
    void (*multiply)(buffer_t *, buffer_t *);
    void (*dub)(buffer_t *, buffer_t *);
    void (*env)(buffer_t *, buffer_t *);
    void (*destroy)(buffer_t *);
} buffer_factory_t;

typedef struct param_factory_t {
    buffer_t * (*from_float)(lpfloat_t);
    buffer_t * (*from_int)(int);
} param_factory_t;

/* Users may create custom memorypools. 
 * If the primary memorypool is active, 
 * it will be used to allocate the pool.
 *
 * Otherwise initializtion of the pool 
 * will use the stdlib to calloc the space.
 */
typedef struct memorypool_t {
    unsigned char * pool;
    size_t poolsize;
    size_t pos;
} memorypool_t;

typedef struct memorypool_factory_t {
    /* This is the primary memorypool. */
    unsigned char * pool;
    size_t poolsize;
    size_t pos;

    void (*init)(unsigned char *, size_t);
    memorypool_t * (*custom_init)(unsigned char *, size_t);
    void * (*alloc)(size_t, size_t);
    void * (*custom_alloc)(memorypool_t *, size_t, size_t);
    void (*free)(void *);
} memorypool_factory_t;

typedef struct interpolation_factory_t {
    lpfloat_t (*linear_pos)(buffer_t*, lpfloat_t);
    lpfloat_t (*linear)(buffer_t*, lpfloat_t);
    lpfloat_t (*hermite_pos)(buffer_t*, lpfloat_t);
    lpfloat_t (*hermite)(buffer_t*, lpfloat_t);
} interpolation_factory_t;

typedef struct wavetable_factory_t {
    buffer_t* (*create)(const char * name, size_t length);
    void (*destroy)(buffer_t*);
} wavetable_factory_t;

typedef struct window_factory_t {
    buffer_t* (*create)(const char * name, size_t length);
    void (*destroy)(buffer_t*);
} window_factory_t;


/* Interfaces */
extern lprand_t Rand;
extern memorypool_factory_t MemoryPool;
extern const buffer_factory_t Buffer;
extern const interpolation_factory_t Interpolation;
extern const param_factory_t Param;
extern const wavetable_factory_t Wavetable;
extern const window_factory_t Window;


/* Utilities */

/* The zapgremlins() routine was written by James McCartney as part of SuperCollider:
 * https://github.com/supercollider/supercollider/blob/f0d4f47a33b57b1f855fe9ca2d4cb427038974f0/headers/plugin_interface/SC_InlineUnaryOp.h#L35
 *
 * SuperCollider real time audio synthesis system
 * Copyright (c) 2002 James McCartney. All rights reserved.
 * http://www.audiosynth.com
 *
 * He says:
 *      This is a function for preventing pathological math operations in ugens.
 *      It can be used at the end of a block to fix any recirculating filter values.
 */
lpfloat_t zapgremlins(lpfloat_t x);

/* This trick came from Hacker's Delight.
 *
 * For values where length is a power of two
 * it is the same as doing:
 *
 *      position = position % length;
 *
 * (But without the division.)
 */
size_t lpfastmod(size_t position, size_t length);

#endif
