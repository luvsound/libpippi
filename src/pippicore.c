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

void destroy_buffer(buffer_t* buf) {
    free(buf->data);
    free(buf);
}

/* Pippi */
buffer_t* mix(buffer_t* a, buffer_t* b) {
    return a;
}

const buffer_factory_t Buffer = { create_buffer, destroy_buffer };
const pippi_factory_t Pippi = { mix };
