#include "pippicore.h"

/* Buffer */
buffer_t * create_buffer(size_t length, int channels, int samplerate) {
    buffer_t * buf;
#ifndef LP_STATIC
    buf = (buffer_t*)calloc(1, sizeof(buffer_t));
    buf->data = (lpfloat_t*)calloc(length * channels, sizeof(lpfloat_t));
#else
    buf = (buffer_t*)MemoryPool.alloc(1, sizeof(buffer_t));
    buf->data = (lpfloat_t*)MemoryPool.alloc(length * channels, sizeof(lpfloat_t));
#endif
    buf->channels = channels;
    buf->length = length;
    buf->samplerate = samplerate;
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
#ifndef LP_STATIC
    free(buf->data);
    free(buf);
#endif
}

/*const buffer_factory_t Buffer = { create_buffer, scale_buffer, mix_buffers, destroy_buffer };*/
const buffer_factory_t Buffer = { create_buffer, scale_buffer, destroy_buffer };
