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

#define DEFAULT_CHANNELS 2
#define DEFAULT_SAMPLERATE 48000

/* Core datatypes */
typedef struct buffer_t {
    lpfloat_t* data;
    size_t length;
    int samplerate;
    int channels;
    lpfloat_t phase;
} buffer_t;

/* Factories */
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

typedef struct memorypool_factory_t {
    unsigned char * pool;
    size_t poolsize;
    size_t pos;

    void (*init)(unsigned char *, size_t);
    void * (*alloc)(size_t, size_t);
    void (*free)(void *);
} memorypool_factory_t;

typedef struct interpolation_factory_t {
    lpfloat_t (*linear_pos)(buffer_t*, lpfloat_t);
    lpfloat_t (*linear)(buffer_t*, lpfloat_t);
    lpfloat_t (*hermite_pos)(buffer_t*, lpfloat_t);
    lpfloat_t (*hermite)(buffer_t*, lpfloat_t);
} interpolation_factory_t;

typedef struct wavetable_factory_t {
    buffer_t* (*create)(char* name, size_t length);
    void (*destroy)(buffer_t*);
} wavetable_factory_t;

typedef struct window_factory_t {
    buffer_t* (*create)(char* name, size_t length);
    void (*destroy)(buffer_t*);
} window_factory_t;


extern const buffer_factory_t Buffer;
extern memorypool_factory_t MemoryPool;
extern const interpolation_factory_t Interpolation;
extern const param_factory_t Param;
extern const wavetable_factory_t Wavetable;
extern const window_factory_t Window;

#endif
