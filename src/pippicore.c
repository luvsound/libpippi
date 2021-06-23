#include "pippicore.h"

#define LOGISTIC_SEED_DEFAULT 3.999
#define LOGISTIC_X_DEFAULT 0.555

#define LORENZ_TIMESTEP_DEFAULT 0.01
#define LORENZ_A_DEFAULT 10.0
#define LORENZ_B_DEFAULT 28.0
#define LORENZ_C_DEFAULT (8.0 / 3.0)
#define LORENZ_X_DEFAULT 0.1
#define LORENZ_Y_DEFAULT 0.0
#define LORENZ_Z_DEFAULT 0.0


/* Forward declarations */
void rand_seed(int value);
lpfloat_t rand_base_logistic(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_stdlib(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_rand(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_lorenz(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_lorenzX(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_lorenzY(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_lorenzZ(lpfloat_t low, lpfloat_t high);
int rand_randint(int low, int high);
int rand_randbool(void);
int rand_choice(int numchoices);

lpbuffer_t * create_buffer(size_t length, int channels, int samplerate);
void scale_buffer(lpbuffer_t * buf, lpfloat_t from_min, lpfloat_t from_max, lpfloat_t to_min, lpfloat_t to_max);
void multiply_buffer(lpbuffer_t * a, lpbuffer_t * b);
void dub_buffer(lpbuffer_t * a, lpbuffer_t * b);
void env_buffer(lpbuffer_t * buf, lpbuffer_t * env);
lpfloat_t play_buffer(lpbuffer_t * buf, lpfloat_t speed);
lpbuffer_t * copy_buffer(lpbuffer_t * buf);
lpbuffer_t * mix_buffers(lpbuffer_t * a, lpbuffer_t * b);
void destroy_buffer(lpbuffer_t * buf);

lpbuffer_t * ringbuffer_create(size_t length, int channels, int samplerate);
void ringbuffer_fill(lpbuffer_t * ringbuf, lpbuffer_t * buf, int offset);
lpfloat_t ringbuffer_readone(lpbuffer_t * ringbuf, int offset);
lpbuffer_t * ringbuffer_read(lpbuffer_t * ringbuf, size_t length);
void ringbuffer_readinto(lpbuffer_t * ringbuf, lpfloat_t * data, int length, int channels);
void ringbuffer_writeone(lpbuffer_t * ringbuf, lpfloat_t sample);
void ringbuffer_writefrom(lpbuffer_t * ringbuf, lpfloat_t * data, int length, int channels);
void ringbuffer_write(lpbuffer_t * ringbuf, lpbuffer_t * buf);
void ringbuffer_dub(lpbuffer_t * buf, lpbuffer_t * src);
void ringbuffer_destroy(lpbuffer_t * buf);

void memorypool_init(unsigned char * pool, size_t poolsize);
lpmemorypool_t * memorypool_custom_init(unsigned char * pool, size_t poolsize);
void * memorypool_alloc(size_t itemcount, size_t itemsize);
void * memorypool_custom_alloc(lpmemorypool_t * pool, size_t itemcount, size_t itemsize);
void memorypool_free(void * ptr);

lpfloat_t interpolate_hermite(lpbuffer_t * buf, lpfloat_t phase);
lpfloat_t interpolate_hermite_pos(lpbuffer_t * buf, lpfloat_t pos);
lpfloat_t interpolate_linear(lpbuffer_t * buf, lpfloat_t phase);
lpfloat_t interpolate_linear_pos(lpbuffer_t * buf, lpfloat_t pos);

lpbuffer_t * param_create_from_float(lpfloat_t value);
lpbuffer_t * param_create_from_int(int value);

lpbuffer_t* create_wavetable(const char * name, size_t length);
void destroy_wavetable(lpbuffer_t* buf);
lpbuffer_t* create_window(const char * name, size_t length);
void destroy_window(lpbuffer_t* buf);

/* Populate interfaces */
lprand_t LPRand = { LOGISTIC_SEED_DEFAULT, LOGISTIC_X_DEFAULT, \
    LORENZ_TIMESTEP_DEFAULT, \
    LORENZ_X_DEFAULT, LORENZ_Y_DEFAULT, LORENZ_Z_DEFAULT, \
    LORENZ_A_DEFAULT, LORENZ_B_DEFAULT, LORENZ_C_DEFAULT, \
    rand_seed, rand_base_logistic, \
    rand_base_lorenz, rand_base_lorenzX, rand_base_lorenzY, rand_base_lorenzZ, \
    rand_base_stdlib, rand_rand, rand_randint, rand_randbool, rand_choice };
lpmemorypool_factory_t LPMemoryPool = { 0, 0, 0, memorypool_init, memorypool_custom_init, memorypool_alloc, memorypool_custom_alloc, memorypool_free };
const lpbuffer_factory_t LPBuffer = { create_buffer, scale_buffer, play_buffer, mix_buffers, multiply_buffer, dub_buffer, env_buffer, destroy_buffer };
const lpinterpolation_factory_t LPInterpolation = { interpolate_linear_pos, interpolate_linear, interpolate_hermite_pos, interpolate_hermite };
const lpparam_factory_t LPParam = { param_create_from_float, param_create_from_int };
const lpwavetable_factory_t LPWavetable = { create_wavetable, destroy_wavetable };
const lpwindow_factory_t LPWindow = { create_window, destroy_window };
const lpringbuffer_factory_t LPRingBuffer = { ringbuffer_create, ringbuffer_fill, ringbuffer_read, ringbuffer_readinto, ringbuffer_writefrom, ringbuffer_write, ringbuffer_readone, ringbuffer_writeone, ringbuffer_dub, ringbuffer_destroy };

/** Rand
 */
void rand_seed(int value) {
    srand((unsigned int)value);
}

lpfloat_t rand_base_stdlib(lpfloat_t low, lpfloat_t high) {
    return (rand()/(lpfloat_t)RAND_MAX) * (high-low) + low;
}

lpfloat_t rand_base_logistic(lpfloat_t low, lpfloat_t high) {
    LPRand.logistic_x = LPRand.logistic_seed * LPRand.logistic_x * (1.f - LPRand.logistic_x);
    return LPRand.logistic_x * (high-low) + low;
}

/* Lorenz attractor implementations lightly adapted with permission from Greg Cope's helpful 
 * overview page: https://www.algosome.com/articles/lorenz-attractor-programming-code.html
 * Please consider Greg Cope's routines to be included here under an MIT license.
 */
lpfloat_t rand_base_lorenzX(lpfloat_t low, lpfloat_t high) {
    LPRand.lorenz_x = LPRand.lorenz_x + LPRand.lorenz_timestep * LPRand.lorenz_a * (LPRand.lorenz_y - LPRand.lorenz_x);
    return LPRand.lorenz_x * (high-low) + low;
}

lpfloat_t rand_base_lorenzY(lpfloat_t low, lpfloat_t high) {
    LPRand.lorenz_y = LPRand.lorenz_y + LPRand.lorenz_timestep * (LPRand.lorenz_x * (LPRand.lorenz_b - LPRand.lorenz_z) - LPRand.lorenz_y);
    return LPRand.lorenz_y * (high-low) + low;
}

lpfloat_t rand_base_lorenzZ(lpfloat_t low, lpfloat_t high) {
    LPRand.lorenz_z = LPRand.lorenz_z + LPRand.lorenz_timestep * (LPRand.lorenz_x * LPRand.lorenz_y - LPRand.lorenz_c * LPRand.lorenz_z);
    return LPRand.lorenz_z * (high-low) + low;
}

/* This will iterate all three lorenz equations and multiply the result. 
 * The individual values are all still available at: 
 *     Rand.lorenz_x, Rand.lorenz_y, Rand.lorenz_z
 */
lpfloat_t rand_base_lorenz(lpfloat_t low, lpfloat_t high) {
    lpfloat_t x, y, z;
    x = rand_base_lorenzX(0.f, 1.f);
    y = rand_base_lorenzY(0.f, 1.f);
    z = rand_base_lorenzZ(0.f, 1.f);
    return x * y * z * (high-low) + low;
}

lpfloat_t rand_rand(lpfloat_t low, lpfloat_t high) {
    return LPRand.rand_base(low, high);
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
lpbuffer_t * create_buffer(size_t length, int channels, int samplerate) {
    lpbuffer_t * buf;
    buf = (lpbuffer_t*)LPMemoryPool.alloc(1, sizeof(lpbuffer_t));
    buf->data = (lpfloat_t*)LPMemoryPool.alloc(length * channels, sizeof(lpfloat_t));
    buf->channels = channels;
    buf->length = length;
    buf->samplerate = samplerate;
    buf->phase = 0.f;
    buf->pos = 0;
    buf->range = length;
    return buf;
}

void scale_buffer(lpbuffer_t * buf, lpfloat_t from_min, lpfloat_t from_max, lpfloat_t to_min, lpfloat_t to_max) {
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

void pan_buffer(lpbuffer_t * buf, lpbuffer_t * pos) {

}

lpfloat_t play_buffer(lpbuffer_t * buf, lpfloat_t speed) {
    lpfloat_t phase_inc, value;
    phase_inc = 1.f / (buf->length * (1.f / speed));
    value = interpolate_linear_pos(buf, buf->phase);
    buf->phase += phase_inc;
    return value;
}

lpbuffer_t * resample_buffer(lpbuffer_t * buf, size_t length) {
    lpbuffer_t * out;
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

void multiply_buffer(lpbuffer_t * a, lpbuffer_t * b) {
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

void env_buffer(lpbuffer_t * buf, lpbuffer_t * env) {
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

void dub_buffer(lpbuffer_t * a, lpbuffer_t * b) {
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

lpbuffer_t * mix_buffers(lpbuffer_t * a, lpbuffer_t * b) {
    int max_channels, max_samplerate, i, c;
    lpbuffer_t * out;
    lpbuffer_t * longest;
    lpbuffer_t * shortest;

    if(a->length >= b->length) {
        longest=a; shortest=b;
    } else {
        longest=b; shortest=a;
    }

    max_channels = (a->channels >= b->channels) ? a->channels : b->channels;
    max_samplerate = (a->samplerate >= b->samplerate) ? a->samplerate : b->samplerate;
    out = LPBuffer.create(longest->length, max_channels, max_samplerate);

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

void destroy_buffer(lpbuffer_t * buf) {
    LPMemoryPool.free(buf->data);
    LPMemoryPool.free(buf);
}

/* RingBuffers
 */
lpbuffer_t * ringbuffer_create(size_t length, int channels, int samplerate) {
    lpbuffer_t * ringbuf;
    ringbuf = LPBuffer.create(length, channels, samplerate);
    ringbuf->pos = 0;
    ringbuf->boundry = length - 1;
    return ringbuf;
}

void ringbuffer_fill(lpbuffer_t * ringbuf, lpbuffer_t * buf, int offset) {
    int i, c;
    size_t pos = ringbuf->pos - buf->length - offset;
    pos = pos % ringbuf->length;
    for(i=0; i < buf->length; i++) {
        for(c=0; c < ringbuf->channels; c++) {
            buf->data[i * buf->channels + c] = ringbuf->data[pos * ringbuf->channels + c];
        }

        pos += 1;
        pos = pos % ringbuf->length;
    }
}

lpfloat_t ringbuffer_readone(lpbuffer_t * ringbuf, int offset) {
    return ringbuf->data[(ringbuf->pos - offset) % ringbuf->length];
}

void ringbuffer_readinto(lpbuffer_t * ringbuf, lpfloat_t * data, int length, int channels) {
    int i, c;
    size_t pos = ringbuf->pos - length;
    pos = pos % ringbuf->length;

    for(i=0; i < length; i++) {
        for(c=0; c < channels; c++) {
            data[i * channels + c] = ringbuf->data[pos * channels + c];
        }

        pos += 1;
        pos = pos % ringbuf->length;
    }
}

lpbuffer_t * ringbuffer_read(lpbuffer_t * ringbuf, size_t length) {
    int i, c;
    size_t pos = ringbuf->pos - length;
    lpbuffer_t * out;

    pos = pos % ringbuf->length;
    out = LPBuffer.create(length, ringbuf->channels, ringbuf->samplerate);
    for(i=0; i < length; i++) {
        for(c=0; c < ringbuf->channels; c++) {
            out->data[i * out->channels + c] = ringbuf->data[pos * ringbuf->channels + c];
        }

        pos += 1;
        pos = pos % ringbuf->length;
    }

    return out;
}

void ringbuffer_writeone(lpbuffer_t * ringbuf, lpfloat_t sample) {
    ringbuf->data[ringbuf->pos] = sample;
    ringbuf->pos += 1;
    ringbuf->pos = ringbuf->pos % ringbuf->length;
}

void ringbuffer_writefrom(lpbuffer_t * ringbuf, lpfloat_t * data, int length, int channels) {
    int i, c, j;
    for(i=0; i < length; i++) {
        for(c=0; c < ringbuf->channels; c++) {
            j = c % channels;
            ringbuf->data[ringbuf->pos * ringbuf->channels + c] = data[i * channels + j];
        }

        ringbuf->pos += 1;
        ringbuf->pos = ringbuf->pos % ringbuf->length;
    }
}

void ringbuffer_write(lpbuffer_t * ringbuf, lpbuffer_t * buf) {
    int i, c, j;
    for(i=0; i < buf->length; i++) {
        for(c=0; c < ringbuf->channels; c++) {
            j = c % buf->channels;
            ringbuf->data[ringbuf->pos * ringbuf->channels + c] = buf->data[i * buf->channels + j];
        }

        ringbuf->pos += 1;
        ringbuf->pos = ringbuf->pos % ringbuf->length;
    }
}

void ringbuffer_dub(lpbuffer_t * buf, lpbuffer_t * src) {
    int i, c, j;
    for(i=0; i < src->length; i++) {
        for(c=0; c < buf->channels; c++) {
            j = c % src->channels;
            buf->data[buf->pos * buf->channels + c] += src->data[i * src->channels + j];
        }

        buf->pos += 1;
        buf->pos = buf->pos % buf->length;
    }
}

void ringbuffer_destroy(lpbuffer_t * buf) {
    LPBuffer.destroy(buf);
}


/* LPMemoryPool
 * */
void memorypool_init(unsigned char * pool, size_t poolsize) {
    assert(poolsize >= 1);
    LPMemoryPool.pool = pool;
    LPMemoryPool.poolsize = poolsize;
    LPMemoryPool.pos = 0;
}

lpmemorypool_t * memorypool_custom_init(unsigned char * pool, size_t poolsize) {
    lpmemorypool_t * mp;
    mp = (lpmemorypool_t *)LPMemoryPool.alloc(1, sizeof(lpmemorypool_t));

    assert(poolsize >= 1);
    mp->pool = pool;
    mp->poolsize = poolsize;
    mp->pos = 0;

    return mp;
}

void * memorypool_custom_alloc(lpmemorypool_t * mp, size_t itemcount, size_t itemsize) {
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

    assert(LPMemoryPool.pool != 0); 
    length = itemcount * itemsize;

    if(LPMemoryPool.poolsize >= LPMemoryPool.pos + length) {
        p = (void *)(&LPMemoryPool.pool[LPMemoryPool.pos]);
        LPMemoryPool.pos += length;
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
lpbuffer_t * param_create_from_float(lpfloat_t value) {
    lpbuffer_t * param = create_buffer(1, 1, DEFAULT_SAMPLERATE);
    param->data[0] = value;
    return param;
}

lpbuffer_t * param_create_from_int(int value) {
    lpbuffer_t * param = create_buffer(1, 1, DEFAULT_SAMPLERATE);
    param->data[0] = (lpfloat_t)value;
    return param;
}

/* Interpolation
 * */
lpfloat_t interpolate_hermite(lpbuffer_t* buf, lpfloat_t phase) {
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

lpfloat_t interpolate_hermite_pos(lpbuffer_t* buf, lpfloat_t pos) {
    return interpolate_hermite(buf, pos * buf->length);
}

lpfloat_t interpolate_linear(lpbuffer_t* buf, lpfloat_t phase) {
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

lpfloat_t interpolate_linear_pos(lpbuffer_t* buf, lpfloat_t pos) {
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
lpbuffer_t* create_wavetable(const char * name, size_t length) {
    lpbuffer_t* buf = LPBuffer.create(length, 1, -1);
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

void destroy_wavetable(lpbuffer_t* buf) {
    LPBuffer.destroy(buf);
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
lpbuffer_t* create_window(const char * name, size_t length) {
    lpbuffer_t* buf = LPBuffer.create(length, 1, -1);
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


void destroy_window(lpbuffer_t* buf) {
    LPBuffer.destroy(buf);
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
lpfloat_t lpzapgremlins(lpfloat_t x) {
    lpfloat_t absx;
    absx = fabs(x);
    return (absx > (lpfloat_t)1e-15 && absx < (lpfloat_t)1e15) ? x : (lpfloat_t)0.f;
}


