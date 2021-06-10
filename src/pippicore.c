#include "pippicore.h"

#define LOGISTIC_SEED_DEFAULT 3.999
#define LOGISTIC_X_DEFAULT 0.555

/* Forward declarations */
void rand_seed(int value);
lpfloat_t rand_base_logistic(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_stdlib(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_rand(lpfloat_t low, lpfloat_t high);
int rand_randint(int low, int high);
int rand_randbool(void);
int rand_choice(int numchoices);

buffer_t * create_buffer(size_t length, int channels, int samplerate);
void scale_buffer(buffer_t * buf, lpfloat_t from_min, lpfloat_t from_max, lpfloat_t to_min, lpfloat_t to_max);
void multiply_buffer(buffer_t * a, buffer_t * b);
void dub_buffer(buffer_t * a, buffer_t * b);
void env_buffer(buffer_t * buf, buffer_t * env);
lpfloat_t play_buffer(buffer_t * buf, lpfloat_t speed);
buffer_t * copy_buffer(buffer_t * buf);
buffer_t * mix_buffers(buffer_t * a, buffer_t * b);
void destroy_buffer(buffer_t * buf);

void memorypool_init(unsigned char * pool, size_t poolsize);
memorypool_t * memorypool_custom_init(unsigned char * pool, size_t poolsize);
void * memorypool_alloc(size_t itemcount, size_t itemsize);
void * memorypool_custom_alloc(memorypool_t * pool, size_t itemcount, size_t itemsize);
void memorypool_free(void * ptr);

lpfloat_t interpolate_hermite(buffer_t* buf, lpfloat_t phase);
lpfloat_t interpolate_hermite_pos(buffer_t* buf, lpfloat_t pos);
lpfloat_t interpolate_linear(buffer_t* buf, lpfloat_t phase);
lpfloat_t interpolate_linear_pos(buffer_t* buf, lpfloat_t pos);

buffer_t * param_create_from_float(lpfloat_t value);
buffer_t * param_create_from_int(int value);

buffer_t* create_wavetable(const char * name, size_t length);
void destroy_wavetable(buffer_t* buf);
buffer_t* create_window(const char * name, size_t length);
void destroy_window(buffer_t* buf);

/* Populate interfaces */
lprand_t Rand = { LOGISTIC_SEED_DEFAULT, LOGISTIC_X_DEFAULT, rand_seed, rand_base_logistic, rand_base_stdlib, rand_rand, rand_randint, rand_randbool, rand_choice };
memorypool_factory_t MemoryPool = { 0, 0, 0, memorypool_init, memorypool_custom_init, memorypool_alloc, memorypool_custom_alloc, memorypool_free };
const buffer_factory_t Buffer = { create_buffer, scale_buffer, play_buffer, mix_buffers, multiply_buffer, dub_buffer, env_buffer, destroy_buffer };
const interpolation_factory_t Interpolation = { interpolate_linear_pos, interpolate_linear, interpolate_hermite_pos, interpolate_hermite };
const param_factory_t Param = { param_create_from_float, param_create_from_int };
const wavetable_factory_t Wavetable = { create_wavetable, destroy_wavetable };
const window_factory_t Window = { create_window, destroy_window };

/** Rand
 */
void rand_seed(int value) {
    srand((unsigned int)value);
}

lpfloat_t rand_base_stdlib(lpfloat_t low, lpfloat_t high) {
    return (rand()/(lpfloat_t)RAND_MAX) * (high-low) + low;
}

lpfloat_t rand_base_logistic(lpfloat_t low, lpfloat_t high) {
    Rand.logistic_x = Rand.logistic_seed * Rand.logistic_x * (1.f - Rand.logistic_x);
    return Rand.logistic_x * (high-low) + low;
}

lpfloat_t rand_rand(lpfloat_t low, lpfloat_t high) {
    return Rand.rand_base(low, high);
}

int rand_randint(int low, int high) {
    float diff, tmp;

    tmp = (float)rand_rand((lpfloat_t)low, (lpfloat_t)high);
    diff = (int)tmp - tmp;

    if(diff >= 0.5f) {
        return (int)ceil(tmp);
    } else {
        return (int)floor(tmp);
    }
}

int rand_randbool(void) {
    return rand_randint(0, 1);
}

int rand_choice(int numchoices) {
    assert(numchoices > 1);
    return rand_randint(0, numchoices-1);
}

/* Buffer
 * */
buffer_t * create_buffer(size_t length, int channels, int samplerate) {
    buffer_t * buf;
    buf = (buffer_t*)MemoryPool.alloc(1, sizeof(buffer_t));
    buf->data = (lpfloat_t*)MemoryPool.alloc(length * channels, sizeof(lpfloat_t));
    buf->channels = channels;
    buf->length = length;
    buf->samplerate = samplerate;
    buf->phase = 0.f;
    buf->pos = 0;
    buf->range = length;
    return buf;
}

void scale_buffer(buffer_t * buf, lpfloat_t from_min, lpfloat_t from_max, lpfloat_t to_min, lpfloat_t to_max) {
    size_t i, c;
    int idx;
    lpfloat_t from_diff, to_diff;

    to_diff = to_max - to_min;;
    from_diff = from_max - from_min;;

    /* Maybe this is valid? It's weird to "scale" 
     * a buffer filled with one value, but I guess 
     * that's a case we should support...
     * Ideally we'll figure out how to get rid of that 
     * repeating divide and use an approach that supports 
     * this case.
     * Anyway: TODO handle this better?
     */
    assert(from_diff != 0);

    for(i=0; i < buf->length; i++) {
        for(c=0; c < buf->channels; c++) {
            idx = i * buf->channels + c;
            buf->data[idx] = ((buf->data[idx] - from_min) / from_diff) * to_diff + to_min;
        }
    }
}

void pan_buffer(buffer_t * buf, buffer_t * pos) {

}

lpfloat_t play_buffer(buffer_t * buf, lpfloat_t speed) {
    lpfloat_t phase_inc, value;
    phase_inc = 1.f / (buf->length * (1.f / speed));
    value = interpolate_linear_pos(buf, buf->phase);
    buf->phase += phase_inc;
    return value;
}

buffer_t * resample_buffer(buffer_t * buf, size_t length) {
    buffer_t * out;
    lpfloat_t pos;
    int i, c;

    assert(length > 1);
    out = create_buffer(length, buf->channels, buf->samplerate);
    for(i=0; i < length; i++) {
        pos = (lpfloat_t)i / length;
        for(c=0; c < buf->channels; c++) {
            out->data[i * buf->channels + c] = play_buffer(buf, pos);
        }
    }

    return out;
}

void multiply_buffer(buffer_t * a, buffer_t * b) {
    size_t length;
    int i, c, j;
    length = (a->length <= b->length) ? a->length : b->length;
    for(i=0; i < length; i++) {
        for(c=0; c < a->channels; c++) {
            j = b->channels % c;
            a->data[i * a->channels + c] *= b->data[i * b->channels + j];
        }
    }
}

void env_buffer(buffer_t * buf, buffer_t * env) {
    lpfloat_t pos, value;
    int i, c;

    assert(env->length > 0);

    for(i=0; i < buf->length; i++) {
        pos = (lpfloat_t)i / buf->length;
        for(c=0; c < buf->channels; c++) {
            value = interpolate_linear_pos(env, pos);
            buf->data[i * buf->channels + c] *= value;
        }
    }
}

void dub_buffer(buffer_t * a, buffer_t * b) {
    size_t length;
    int i, c, j;
    length = (a->length <= b->length) ? a->length : b->length;
    for(i=0; i < length; i++) {
        for(c=0; c < a->channels; c++) {
            j = b->channels % c;
            a->data[i * a->channels + c] += b->data[i * b->channels + j];
        }
    }
}

buffer_t * mix_buffers(buffer_t * a, buffer_t * b) {
    int max_channels, max_samplerate, i, c;
    buffer_t * out;
    buffer_t * longest;
    buffer_t * shortest;

    if(a->length >= b->length) {
        longest=a; shortest=b;
    } else {
        longest=b; shortest=a;
    }

    max_channels = (a->channels >= b->channels) ? a->channels : b->channels;
    max_samplerate = (a->samplerate >= b->samplerate) ? a->samplerate : b->samplerate;
    out = Buffer.create(longest->length, max_channels, max_samplerate);

    for(i=0; i < longest->length; i++) {
        for(c=0; c < max_channels; c++) {
            out->data[i * max_channels + c] = longest->data[i * max_channels + c];
        }
    }

    for(i=0; i < shortest->length; i++) {
        for(c=0; c < max_channels; c++) {
            out->data[i * max_channels + c] = shortest->data[i * max_channels + c];
        }
    }

    return out;
}

void destroy_buffer(buffer_t * buf) {
    MemoryPool.free(buf->data);
    MemoryPool.free(buf);
}

/* MemoryPool
 * */
void memorypool_init(unsigned char * pool, size_t poolsize) {
    assert(poolsize >= 1);
    MemoryPool.pool = pool;
    MemoryPool.poolsize = poolsize;
    MemoryPool.pos = 0;
}

memorypool_t * memorypool_custom_init(unsigned char * pool, size_t poolsize) {
    memorypool_t * mp;
    mp = (memorypool_t *)MemoryPool.alloc(1, sizeof(memorypool_t));

    assert(poolsize >= 1);
    mp->pool = pool;
    mp->poolsize = poolsize;
    mp->pos = 0;

    return mp;
}

void * memorypool_custom_alloc(memorypool_t * mp, size_t itemcount, size_t itemsize) {
    void * p;
    size_t length;

    assert(mp->pool != 0); 
    length = itemcount * itemsize;

    if(mp->poolsize >= mp->pos + length) {
        p = (void *)(&mp->pool[mp->pos]);
        mp->pos += length;
        return p;
    }
    /* FIXME might as well try to expand the pool here */
    exit(EXIT_FAILURE);
}

void * memorypool_alloc(size_t itemcount, size_t itemsize) {
#ifdef LP_STATIC
    void * p;
    size_t length;

    assert(MemoryPool.pool != 0); 
    length = itemcount * itemsize;

    if(MemoryPool.poolsize >= MemoryPool.pos + length) {
        p = (void *)(&MemoryPool.pool[MemoryPool.pos]);
        MemoryPool.pos += length;
        return p;
    }
    exit(EXIT_FAILURE);
#else
    return calloc(itemcount, itemsize);
#endif
}

void memorypool_free(void * ptr) {
#ifndef LP_STATIC
    free(ptr);
#endif
}

/* Param
 * */
buffer_t * param_create_from_float(lpfloat_t value) {
    buffer_t * param = create_buffer(1, 1, DEFAULT_SAMPLERATE);
    param->data[0] = value;
    return param;
}

buffer_t * param_create_from_int(int value) {
    buffer_t * param = create_buffer(1, 1, DEFAULT_SAMPLERATE);
    param->data[0] = (lpfloat_t)value;
    return param;
}

/* Interpolation
 * */
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

    if(buf->length == 1) return buf->data[0];
    
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
buffer_t* create_wavetable(const char * name, size_t length) {
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

char * random_window(void) {
    char * name;
    int choice = rand_choice(4);
    switch (choice) {
        case 0:
            name = (char *)PHASOR;
            break;

        case 1:
            name = (char *)TRI;
            break;

        case 2:
            name = (char *)SINE;
            break;

        case 3:
            name = (char *)HANN;
            break;

        default:
            name = (char *)SINE;
    }

    return name;
}


/* create a window (0 to 1) */
buffer_t* create_window(const char * name, size_t length) {
    buffer_t* buf = Buffer.create(length, 1, -1);
    if(strcmp(name, SINE) == 0) {
        window_sine(buf->data, length);            
    } else if (strcmp(name, TRI) == 0) {
        window_tri(buf->data, length);            
    } else if (strcmp(name, PHASOR) == 0) {
        window_phasor(buf->data, length);            
    } else if (strcmp(name, HANN) == 0) {
        window_hanning(buf->data, length);            
    } else if (strcmp(name, RND) == 0) {
        return create_window(random_window(), length);
    } else {
        window_sine(buf->data, length);            
    }
    return buf;
}


void destroy_window(buffer_t* buf) {
    Buffer.destroy(buf);
}

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
lpfloat_t zapgremlins(lpfloat_t x) {
    lpfloat_t absx;
    absx = fabs(x);
    return (absx > (lpfloat_t)1e-15 && absx < (lpfloat_t)1e15) ? x : (lpfloat_t)0.f;
}


