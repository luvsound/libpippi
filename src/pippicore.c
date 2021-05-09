#include "pippicore.h"

/* Buffer */
buffer_t* create_buffer(size_t length, int channels, int samplerate) {
    buffer_t* buf = (buffer_t*)calloc(1, sizeof(buffer_t));
    buf->data = (lpfloat_t*)calloc(length * channels, sizeof(lpfloat_t));
    buf->channels = channels;
    buf->length = length;
    buf->samplerate = samplerate;
    return buf;
}

void scale_buffer(buffer_t* buf, lpfloat_t from_min, lpfloat_t from_max, lpfloat_t to_min, lpfloat_t to_max) {
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

buffer_t* mix_buffers(int count, ...) {
    va_list ap;
    buffer_t* buf;
    buffer_t* out;
    size_t max_length;
    int max_channels, max_samplerate, i, j, c;

    max_length = 0;
    max_channels = 0;
    max_samplerate = 0;

    va_start(ap, count);
    for(i=0; i < count; i++) {
        buf = va_arg(ap, buffer_t*);
        if(buf->length > max_length) max_length = buf->length;
        if(buf->channels > max_channels) max_channels = buf->channels;
        if(buf->samplerate > max_samplerate) max_samplerate = buf->samplerate;
    }
    va_end(ap);

    out = Buffer.create(max_length, max_channels, max_samplerate);

    va_start(ap, count);
    for(i=0; i < count; i++) {
        buf = va_arg(ap, buffer_t*);
        for(j=0; j < max_length; j++) {
            if(j >= buf->length) break;
            for(c=0; c < max_channels; c++) {
                out->data[i * max_channels + c] = buf->data[j * max_channels + c];
            }
        }
    }
    va_end(ap);

    return out;
}

void destroy_buffer(buffer_t* buf) {
    free(buf->data);
    free(buf);
}

/*const buffer_factory_t Buffer = { create_buffer, scale_buffer, mix_buffers, destroy_buffer };*/
const buffer_factory_t Buffer = { create_buffer, scale_buffer, destroy_buffer };
