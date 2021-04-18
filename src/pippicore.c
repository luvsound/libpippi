#include "pippicore.h"

static buffer_t* init_buffer(size_t length, int channels, int samplerate) {
    buffer_t* buf = (buffer_t*)calloc(1, sizeof(buffer_t));
    buf->data = (lpfloat_t*)calloc(length * channels, sizeof(lpfloat_t));
    buf->channels = channels;
    buf->length = length;
    buf->samplerate = samplerate;
    return buf;
}

static void destroy_buffer(buffer_t* buf) {
    free(buf->data);
    free(buf);
}

const pippi_factory_t Pippi = { init_buffer, destroy_buffer };

