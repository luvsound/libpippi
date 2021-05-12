#include "pippicore.h"

/* Forward declarations */
buffer_t * create_buffer(size_t length, int channels, int samplerate);
void scale_buffer(buffer_t * buf, lpfloat_t from_min, lpfloat_t from_max, lpfloat_t to_min, lpfloat_t to_max);
lpfloat_t read_buffer(buffer_t * buf, lpfloat_t speed);
buffer_t * mix_buffers(buffer_t * a, buffer_t * b);
void destroy_buffer(buffer_t * buf);

void * memorypool_alloc(size_t itemcount, size_t itemsize);
void memorypool_init(unsigned char * pool, size_t poolsize);
void memorypool_free(void * ptr);

lpfloat_t interpolate_hermite(buffer_t* buf, lpfloat_t phase);
lpfloat_t interpolate_hermite_pos(buffer_t* buf, lpfloat_t pos);
lpfloat_t interpolate_linear(buffer_t* buf, lpfloat_t phase);
lpfloat_t interpolate_linear_pos(buffer_t* buf, lpfloat_t pos);

/* Populate interfaces */
memorypool_factory_t MemoryPool = { 0, 0, 0, memorypool_init, memorypool_alloc, memorypool_free };
const buffer_factory_t Buffer = { create_buffer, scale_buffer, read_buffer, mix_buffers, destroy_buffer };
const interpolation_factory_t Interpolation = { interpolate_linear_pos, interpolate_linear, interpolate_hermite_pos, interpolate_hermite };


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

lpfloat_t read_buffer(buffer_t * buf, lpfloat_t speed) {
    lpfloat_t phase_inc, value;
    phase_inc = 1.f / (buf->length * (1.f / speed));
    value = interpolate_linear_pos(buf, buf->phase);
    buf->phase += phase_inc;
    return value;
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
